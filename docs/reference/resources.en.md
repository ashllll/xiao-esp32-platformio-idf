# Resource index

The project keeps concise reviewed guidance in the repository and large PDFs, schematics, CAD, archives, and vendor mirrors in an optional external library.

## Optional local library

```bash
export XIAO_ESP32_REFERENCE_ROOT="${XIAO_ESP32_REFERENCE_ROOT:-$HOME/ESP32_资料整理}"
```

The library is not a build dependency. Treat its `README.md` as the curated entry point. Historical generated notes, recovery folders, caches, and binaries are discovery evidence only, not authoritative implementation sources.

## First-party board sources

| Board | Seeed guide | PlatformIO board page |
|---|---|---|
| XIAO ESP32C3 | [Getting started](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/) | [Manifest docs](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32c3.html) |
| XIAO ESP32S3 | [Getting started](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) | [Manifest docs](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32s3.html) |
| XIAO ESP32C6 | [Getting started](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/) | [Manifest docs](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32c6.html) |

## Toolchain and API sources

- [PlatformIO Espressif 32](https://registry.platformio.org/platforms/platformio/espressif32)
- [ESP-IDF 6.0.1 Programming Guide](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/)
- [Espressif Component Registry](https://components.espressif.com/)
- [MkDocs](https://www.mkdocs.org/)
- [Material for MkDocs](https://squidfunk.github.io/mkdocs-material/)

The repository baseline is PlatformIO Core 6.1.19, Espressif 32 Platform 7.0.1, and ESP-IDF 6.0.1. A `stable` documentation URL can move to a later patch; version-sensitive examples should use 6.0.1 and be rechecked during upgrades.

## Corrected board details

- C3 GPIO2/8/9 are strap-related.
- S3 native USB uses GPIO19/20.
- S3 Sense microSD uses GPIO3/7/8/9 (CS/SCK/MISO/MOSI), not GPIO21.
- S3 GPIO21 is the active-low user LED.
- C6 has a single HP RISC-V CPU plus an LP RISC-V core and integrated IEEE 802.15.4 hardware.

Last reviewed: 2026-07-22.
