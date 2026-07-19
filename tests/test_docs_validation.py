from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest

from scripts.validate_docs import validate_project_docs


class DocumentationValidationTests(unittest.TestCase):
    def make_project(self, root: Path) -> Path:
        (root / "docs" / "hardware").mkdir(parents=True)
        (root / "docs" / "firmware").mkdir(parents=True)
        (root / "docs" / "reference").mkdir(parents=True)
        baseline = {
            "esp_idf": "6.0.1",
            "mkdocs": "1.6.1",
            "mkdocs_material": "9.7.7",
            "platformio_core": "6.1.19",
            "platformio_espressif32": "7.0.1",
            "project_version": "0.3.0",
            "verified_on": "2026-07-18",
        }
        (root / "project-baseline.json").write_text(json.dumps(baseline), encoding="utf-8")
        (root / "platformio.ini").write_text(
            "[env]\nplatform = espressif32@7.0.1\n", encoding="utf-8"
        )
        (root / "requirements-dev.txt").write_text("platformio==6.1.19\n", encoding="utf-8")
        (root / "requirements-docs.txt").write_text(
            "mkdocs==1.6.1\nmkdocs-material==9.7.7\n", encoding="utf-8"
        )
        (root / "CMakeLists.txt").write_text('set(PROJECT_VER "0.3.0")\n', encoding="utf-8")
        (root / "CHANGELOG.md").write_text("# Changelog\n\n## [0.3.0]\n", encoding="utf-8")
        documents = {
            "index.md": "# 首页\n\n基线 7.0.1 / 6.0.1。\n\n[板卡](hardware/boards.md#矩阵)\n",
            "hardware/boards.md": "# 板卡\n\n## 矩阵\n\n基线 7.0.1 / 6.0.1。\n",
            "firmware/configuration.md": "# 配置\n\n基线 7.0.1 / 6.0.1。\n",
            "reference/resources.md": "# 资料\n\n基线 7.0.1 / 6.0.1。\n",
        }
        for relative, contents in documents.items():
            path = root / "docs" / relative
            path.write_text(contents, encoding="utf-8")
        (root / "README.md").write_text(
            "# Project\n\n当前源码版本为 0.3.0。\n\n[Docs](docs/index.md)\n",
            encoding="utf-8",
        )
        (root / "mkdocs.yml").write_text(
            "nav:\n"
            "  - index.md\n"
            "  - hardware/boards.md\n"
            "  - firmware/configuration.md\n"
            "  - reference/resources.md\n",
            encoding="utf-8",
        )
        return root

    def test_valid_project_passes(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            project = self.make_project(Path(directory))
            self.assertEqual(validate_project_docs(project), [])

    def test_stale_version_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            project = self.make_project(Path(directory))
            path = project / "docs" / "index.md"
            path.write_text(path.read_text() + "ESP-IDF 6.0.0\n", encoding="utf-8")
            self.assertTrue(any("stale baseline" in error for error in validate_project_docs(project)))

    def test_document_versions_follow_baseline(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            project = self.make_project(Path(directory))
            baseline_path = project / "project-baseline.json"
            baseline = json.loads(baseline_path.read_text(encoding="utf-8"))
            baseline["platformio_espressif32"] = "9.9.9"
            baseline["esp_idf"] = "8.8.8"
            baseline_path.write_text(json.dumps(baseline), encoding="utf-8")
            errors = validate_project_docs(project)
            self.assertTrue(any("9.9.9" in error for error in errors))
            self.assertTrue(any("8.8.8" in error for error in errors))

    def test_broken_link_and_anchor_fail(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            project = self.make_project(Path(directory))
            path = project / "docs" / "index.md"
            path.write_text(
                path.read_text() + "[Missing](missing.md) [Anchor](hardware/boards.md#missing)\n",
                encoding="utf-8",
            )
            errors = validate_project_docs(project)
            self.assertTrue(any("broken relative link" in error for error in errors))
            self.assertTrue(any("missing anchor" in error for error in errors))

    def test_nav_omission_and_placeholder_fail(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            project = self.make_project(Path(directory))
            extra = project / "docs" / "extra.md"
            extra.write_text("# <外设名称>\n", encoding="utf-8")
            errors = validate_project_docs(project)
            self.assertTrue(any("nav omits" in error for error in errors))
            self.assertTrue(any("placeholder" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
