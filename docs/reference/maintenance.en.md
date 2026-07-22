# Documentation maintenance

## Trust order

Prefer current Seeed board documentation and schematics, Espressif versioned documentation/source, PlatformIO release/board manifests, and component-vendor datasheets. Derived notes and local mirrors can help discovery but do not outrank their owning source.

## Updating a technical claim

1. Identify the exact board/module and version-sensitive framework context.
2. Open the first-party source and record its URL and review date.
3. Update Chinese and English pages from the same fact table.
4. Preserve evidence labels: specification, build result, hardware result, and calibrated measurement are different.
5. Run offline validation, unit tests, and a strict bilingual site build.

## Version changes

Keep `project-baseline.json`, `platformio.ini`, requirements, `CHANGELOG.md`, configuration pages, and both language variants synchronized. Rebuild all three environments after a PlatformIO or ESP-IDF change.

## External library

Use `XIAO_ESP32_REFERENCE_ROOT` for large source archives. Do not make it required for CI, copy it wholesale into the repository, or use quarantined/recovery material as an implementation source. Preserve provenance when replacing a local official mirror.

## Translation rules

- Every published base Markdown file requires a matching `.en.md` file.
- `fallback_to_default` remains disabled.
- Paths, code, GPIO numbers, units, API names, version numbers, and evidence status must remain semantically identical.
- Navigation titles and site metadata are localized in `mkdocs.yml`.
- New research records enter navigation and receive an English counterpart in the same change.

Run:

```bash
python3 scripts/validate_docs.py
python3 -m unittest discover -s tests -p 'test_*.py'
mkdocs build --strict
```
