#!/usr/bin/env python3
"""Validate documentation structure and version consistency without network access."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import sys
from urllib.parse import unquote


PUBLISHED_ROOT_MARKDOWN = ("README.md", "CONTRIBUTING.md", "SECURITY.md", "CHANGELOG.md")
EXCLUDED_DOCS = {"peripherals/_template.md"}
STALE_BASELINES = (
    "espressif32@7.0.0",
    "Espressif32 Platform 7.0.0",
    "Espressif 32 Platform 7.0.0",
    "ESP-IDF 6.0.0",
    "本工程基线是 6.0.0",
)
LEAKED_PLACEHOLDERS = (
    "<外设名称>",
    "<占位符>",
    "YYYY-MM-DD",
)
LINK_RE = re.compile(r"!?\[[^\]]*\]\(([^)]+)\)")
HEADING_RE = re.compile(r"^#{1,6}\s+(.+?)\s*$", re.MULTILINE)
EXPLICIT_ANCHOR_RE = re.compile(r"\{\s*#([A-Za-z0-9_.:-]+)\s*\}\s*$")


class DocumentationError(ValueError):
    """Raised when documentation is inconsistent or incomplete."""


def _read_json(path: Path) -> dict[str, str]:
    data = json.loads(path.read_text(encoding="utf-8"))
    required = {
        "esp_idf",
        "mkdocs",
        "mkdocs_material",
        "platformio_core",
        "platformio_espressif32",
        "project_version",
        "verified_on",
    }
    missing = sorted(required - data.keys())
    if missing:
        raise DocumentationError("project-baseline.json is missing: " + ", ".join(missing))
    return {key: str(value) for key, value in data.items()}


def _require_text(path: Path, expected: str, errors: list[str]) -> None:
    if not path.is_file():
        errors.append(f"missing file: {path}")
        return
    if expected not in path.read_text(encoding="utf-8"):
        errors.append(f"{path}: missing expected text {expected!r}")


def validate_versions(project: Path, errors: list[str]) -> dict[str, str]:
    baseline_path = project / "project-baseline.json"
    if not baseline_path.is_file():
        errors.append(f"missing file: {baseline_path}")
        return {}
    try:
        baseline = _read_json(baseline_path)
    except (json.JSONDecodeError, DocumentationError) as error:
        errors.append(f"{baseline_path}: {error}")
        return {}

    platform = baseline["platformio_espressif32"]
    _require_text(project / "platformio.ini", f"platform = espressif32@{platform}", errors)
    _require_text(
        project / "requirements-dev.txt",
        f"platformio=={baseline['platformio_core']}",
        errors,
    )
    _require_text(project / "requirements-docs.txt", f"mkdocs=={baseline['mkdocs']}", errors)
    _require_text(
        project / "requirements-docs.txt",
        f"mkdocs-material=={baseline['mkdocs_material']}",
        errors,
    )
    _require_text(
        project / "CMakeLists.txt",
        f'set(PROJECT_VER "{baseline["project_version"]}")',
        errors,
    )
    _require_text(project / "CHANGELOG.md", f"## [{baseline['project_version']}]", errors)
    _require_text(project / "README.md", f"当前源码版本为 {baseline['project_version']}", errors)

    baseline_docs = (
        project / "docs" / "index.md",
        project / "docs" / "hardware" / "boards.md",
        project / "docs" / "firmware" / "configuration.md",
        project / "docs" / "reference" / "resources.md",
    )
    for path in baseline_docs:
        _require_text(path, platform, errors)
        _require_text(path, baseline["esp_idf"], errors)
    return baseline


def _slugify_heading(heading: str) -> str:
    explicit = EXPLICIT_ANCHOR_RE.search(heading)
    if explicit:
        return explicit.group(1)
    heading = re.sub(r"\{[^}]*\}\s*$", "", heading)
    heading = re.sub(r"`([^`]*)`", r"\1", heading)
    heading = re.sub(r"\[([^\]]+)\]\([^)]*\)", r"\1", heading)
    heading = heading.strip().lower()
    heading = re.sub(r"[^\w\-\s\u0080-\uffff]", "", heading)
    return re.sub(r"[\s_]+", "-", heading).strip("-")


def _anchors(path: Path) -> set[str]:
    return {_slugify_heading(value) for value in HEADING_RE.findall(path.read_text(encoding="utf-8"))}


def _markdown_files(project: Path) -> list[Path]:
    files = [project / name for name in PUBLISHED_ROOT_MARKDOWN if (project / name).is_file()]
    docs = project / "docs"
    if docs.is_dir():
        files.extend(
            path
            for path in sorted(docs.rglob("*.md"))
            if path.relative_to(docs).as_posix() not in EXCLUDED_DOCS
        )
    return files


def validate_markdown(project: Path, errors: list[str]) -> None:
    anchors: dict[Path, set[str]] = {}
    for source in _markdown_files(project):
        text = source.read_text(encoding="utf-8")
        for stale in STALE_BASELINES:
            if stale in text:
                errors.append(f"{source}: stale baseline {stale!r}")
        for placeholder in LEAKED_PLACEHOLDERS:
            if placeholder in text:
                errors.append(f"{source}: leaked template placeholder {placeholder!r}")
        if "/Users/" in text:
            errors.append(f"{source}: contains a personal absolute path")

        for raw_target in LINK_RE.findall(text):
            target = raw_target.strip().strip("<>")
            if not target or re.match(r"^(?:https?|mailto|tel):", target):
                continue
            target = unquote(target)
            path_text, _, fragment = target.partition("#")
            destination = source if not path_text else (source.parent / path_text).resolve()
            if destination.is_dir():
                destination = destination / "README.md"
            if not destination.is_file():
                errors.append(f"{source}: broken relative link {raw_target!r}")
                continue
            if fragment and destination.suffix.lower() == ".md":
                destination = destination.resolve()
                if destination not in anchors:
                    anchors[destination] = _anchors(destination)
                if fragment not in anchors[destination]:
                    errors.append(f"{source}: missing anchor #{fragment} in {destination}")


def validate_mkdocs_nav(project: Path, errors: list[str]) -> None:
    config = project / "mkdocs.yml"
    docs = project / "docs"
    if not config.is_file() or not docs.is_dir():
        errors.append("mkdocs.yml or docs/ is missing")
        return
    text = config.read_text(encoding="utf-8")
    referenced = set(re.findall(r"(?:^|\s)([A-Za-z0-9_./-]+\.md)\s*$", text, re.MULTILINE))
    published = {
        path.relative_to(docs).as_posix()
        for path in docs.rglob("*.md")
        if path.relative_to(docs).as_posix() not in EXCLUDED_DOCS
    }
    for relative in sorted(published - referenced):
        errors.append(f"mkdocs.yml nav omits published document: {relative}")
    for relative in sorted(referenced - published - EXCLUDED_DOCS):
        errors.append(f"mkdocs.yml references missing document: {relative}")


def validate_project_docs(project: Path) -> list[str]:
    errors: list[str] = []
    validate_versions(project, errors)
    validate_markdown(project, errors)
    validate_mkdocs_nav(project, errors)
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("project", nargs="?", type=Path, default=Path.cwd())
    args = parser.parse_args()
    project = args.project.expanduser().resolve()
    errors = validate_project_docs(project)
    if errors:
        for error in errors:
            print(f"error: {error}", file=sys.stderr)
        print(f"Documentation validation failed with {len(errors)} error(s).", file=sys.stderr)
        return 1
    print(f"Documentation validation passed: {project}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
