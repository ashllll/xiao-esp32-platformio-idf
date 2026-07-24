# Waveshare ESP32-C6-LCD-1.47

Sources reviewed: 2026-07-25

This directory covers the Waveshare `ESP32-C6-LCD-1.47` (SKU 28563) and the pin-header `-M` variant (SKU 30381). They are not the Seeed XIAO ESP32C6 and must not be confused with the touch, 8 MB Flash, IMU-equipped `ESP32-C6-Touch-LCD-1.47`.

!!! warning "Protect the display"
    Waveshare says to keep backlight brightness at **50% or below** and avoid prolonged full-brightness use. Heat can produce a dark shadow at the bottom. Power down and cool an affected board, then flash firmware using lower brightness. Do not remove the screen when soldering headers.

## Verified specification

| Item | Result |
|---|---|
| MCU | ESP32-C6FH4; HP RISC-V core up to 160 MHz and LP core up to 20 MHz |
| Memory | 4 MB in-package Flash, 512 KB HP SRAM, 16 KB LP SRAM, 320 KB ROM |
| Radio | 2.4 GHz Wi-Fi 6, Bluetooth 5 LE, IEEE 802.15.4 |
| Display | 1.47-inch 172×320 262K-color TFT, ST7789 family, 4-wire SPI |
| Storage | Onboard microSD/TF slot in SPI mode |
| USB | USB-C connected to ESP32-C6 USB Serial/JTAG |
| Other | Addressable RGB LED on GPIO8, BOOT and RESET buttons, ceramic antenna |

The panel advertises 262K colors, while the vendor examples transfer RGB565. A full buffer is `172×320×2 = 110,080` bytes; the vendor ESP-IDF example uses a 20-row partial buffer rather than two full framebuffers.

## Guide

1. [Hardware and pinout](hardware.md)
2. [Development and examples](development.md)
3. [Troubleshooting and acceptance](troubleshooting.md)
4. The primary-source review records discrepancies between the schematic, download and web pages.

## Scope

- This repository's `xiao_esp32c6` board definition and pins are for XIAO only.
- The vendor bundle contains separate Arduino and native ESP-IDF projects; Arduino APIs do not belong in this repository's pure ESP-IDF application.
- Sources and builds can be checked without hardware, but they do not prove display, card, RF or thermal behavior.

## Official entry points

- [Current Waveshare guide](https://docs.waveshare.com/ESP32-C6-LCD-1.47)
- [Board schematic](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf)
- [Vendor example bundle](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47-Demo.zip)
- [ESP32-C6 datasheet](https://documentation.espressif.com/esp32-c6_datasheet_en.html)
- [ESP-IDF ESP32-C6 guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/)
