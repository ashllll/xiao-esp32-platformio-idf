#!/usr/bin/env python3
"""Convert one PEM certificate into the local ikuai_cert.h representation."""

from pathlib import Path
import argparse


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    pem = args.input.read_text(encoding="ascii").strip()
    if not pem.startswith("-----BEGIN CERTIFICATE-----"):
        raise SystemExit("input is not a PEM certificate")
    if not pem.endswith("-----END CERTIFICATE-----"):
        raise SystemExit("PEM certificate is incomplete")

    lines = ["#pragma once", "", "static const char ikuai_cert_pem[] ="]
    lines.extend(f'    "{line}\\n"' for line in pem.splitlines())
    lines[-1] += ";"
    args.output.write_text("\n".join(lines) + "\n", encoding="ascii")


if __name__ == "__main__":
    main()
