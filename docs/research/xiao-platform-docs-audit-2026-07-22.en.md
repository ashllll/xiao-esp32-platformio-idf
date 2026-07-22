# XIAO / PlatformIO / ESP-IDF documentation audit

Review date: 2026-07-22

This audit checks the repository's core board, toolchain, and documentation-site claims against first-party Seeed Studio, Espressif, PlatformIO, MkDocs, and Material for MkDocs sources.

## Executive summary

- **Confirmed:** the C3/S3/C6 D0-D10 mappings, PlatformIO board IDs, Flash/PSRAM table, and the PlatformIO Espressif 32 7.0.1 to ESP-IDF 6.0.1 relationship.
- **Corrected:** S3 Sense microSD uses CS=GPIO3, SCK=GPIO7, MISO=GPIO8, and MOSI=GPIO9. GPIO21 is the user LED, not SD chip-select.
- **Corrected:** ESP32-C6 includes a single high-performance RISC-V CPU and a low-power RISC-V core; “single-core RISC-V” alone is incomplete.
- **Clarified:** early S3 Sense units used OV2640, later units use OV3660, and Seeed documents OV5640 as a compatible replacement.
- **Site boundary:** setting `theme.language` localizes one Material build; it does not translate content. The i18n layer therefore provides separate URLs, page-level language metadata, localized navigation/search, and strict bilingual parity.

## Claim review

| Claim | Result | First-party evidence |
|---|---|---|
| `espressif32@7.0.1` integrates ESP-IDF 6.0.1 | Confirmed | [Platform release](https://github.com/platformio/platform-espressif32/releases/tag/v7.0.1) |
| Board IDs are `seeed_xiao_esp32c3`, `seeed_xiao_esp32s3`, and `seeed_xiao_esp32c6` | Confirmed | [C3](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32c3.html), [S3](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32s3.html), [C6](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32c6.html) |
| C3/C6 have 4 MB Flash; base S3 has 8 MB Flash and 8 MB Octal PSRAM | Confirmed; S3 Plus is a different 16 MB Flash variant | [C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/), [S3](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/), [C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/) |
| C3/S3/C6 D0-D10 table | Confirmed | The three Seeed pin maps above |
| C6 is only “single-core RISC-V” | Incomplete; use single HP CPU + LP RISC-V core | [Seeed C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/), [Espressif datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf) |
| S3 Sense SD uses GPIO7/8/9/21 | Incorrect; CS is GPIO3, not GPIO21 | [Seeed S3 Sense pin map](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#hardware-overview) |
| S3 user LED is active-low GPIO21 | Confirmed | [Seeed S3 FreeRTOS guide](https://wiki.seeedstudio.com/xiao-esp32s3-freertos/) |
| C3 strap cautions on GPIO2/8/9 | Confirmed | [Seeed C3 strap pins](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#strapping-pins) |
| S3 native USB uses GPIO19/20 | Confirmed | [ESP-IDF 6.0.1 USB Serial/JTAG](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-guides/usb-serial-jtag-console.html) |

## Bilingual-site acceptance

1. Chinese is built at `/` and English at `/en/`.
2. Every published page has both source files; fallback is disabled.
3. Each rendered page uses the matching HTML `lang` and `hreflang` alternates.
4. Navigation, site metadata, theme labels, and search are localized.
5. Language switching remains on the same topic when both pages exist.
6. CI runs offline parity/link/anchor checks and `mkdocs build --strict` for both languages.

Official implementation references: [Material language guidance](https://squidfunk.github.io/mkdocs-material/setup/changing-the-language/), [MkDocs configuration](https://www.mkdocs.org/user-guide/configuration/), and [MkDocs CLI](https://www.mkdocs.org/user-guide/cli/).

## Evidence boundary

These sources establish official specifications and supported build targets. They do not prove that a physical device ran the current commit, that a deployed Pages site matches this workspace, or that a third-party peripheral works under a particular wiring and load. Those claims require runtime, deployment, or hardware evidence respectively.
