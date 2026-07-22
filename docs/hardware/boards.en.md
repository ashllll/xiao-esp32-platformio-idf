# XIAO board matrix

## Supported boards

| PlatformIO environment | Board manifest | CPU family | Flash | PSRAM | Integrated radios |
|---|---|---|---:|---:|---|
| `xiao_esp32c3` | `seeed_xiao_esp32c3` | RISC-V | 4 MB | none onboard | 2.4 GHz Wi-Fi, BLE |
| `xiao_esp32s3` | `seeed_xiao_esp32s3` | Xtensa | 8 MB | Octal PSRAM | 2.4 GHz Wi-Fi, BLE |
| `xiao_esp32c6` | `seeed_xiao_esp32c6` | 32-bit RISC-V; single HP CPU + LP RISC-V core | 4 MB | none onboard | 2.4 GHz Wi-Fi 6, BLE, IEEE 802.15.4 |

The build baseline is PlatformIO Espressif 32 7.0.1 with its integrated ESP-IDF 6.0.1.

## Variant boundaries

- XIAO ESP32S3, S3 Sense, and S3 Plus do not share every expansion-pin assignment.
- The S3 Sense microSD interface uses CS=GPIO3, SCK=GPIO7, MISO=GPIO8, and MOSI=GPIO9, conflicting with D2/D8/D9/D10. GPIO21 is the user LED, not SD chip-select.
- Early S3 Sense cameras used the discontinued OV2640; later boards use OV3660, with OV5640 documented as a compatible replacement. Record the actual revision.
- C6 IEEE 802.15.4 capability does not by itself prove that a particular Zigbee, Thread, or Matter configuration works with this project baseline.

The firmware validates chip family, Flash size, and S3 PSRAM. It cannot authenticate the board vendor or identify every expansion-board revision.

## Evidence status

All three environments are compile-tested. Build results are not hardware results; see the [hardware validation matrix](validation.md) for current device evidence.

## Official sources

- [Seeed XIAO ESP32C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)
- [Seeed XIAO ESP32S3](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- [Seeed XIAO ESP32C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/)
- [PlatformIO Espressif 32](https://registry.platformio.org/platforms/platformio/espressif32)

Last reviewed: 2026-07-22.
