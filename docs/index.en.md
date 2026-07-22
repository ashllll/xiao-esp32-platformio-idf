# XIAO ESP32 PlatformIO + ESP-IDF Guide

This site documents a reproducible ESP-IDF project for Seeed Studio XIAO ESP32C3, ESP32S3, and ESP32C6 boards. It covers board selection, pin mapping, power constraints, firmware configuration, peripheral integration, hardware acceptance, and releases.

## Verified baseline

- PlatformIO Core 6.1.19
- PlatformIO Espressif 32 7.0.1
- ESP-IDF 6.0.1, as integrated by the pinned PlatformIO platform
- MkDocs 1.6.1, Material 9.7.7, and mkdocs-static-i18n 1.3.1
- Last baseline verification: 2026-07-22

The repository distinguishes three evidence levels:

1. **Build verified** means the selected toolchain produced firmware.
2. **Hardware verified** means the exact firmware revision passed the runtime contract on a named board.
3. **Measurement validated** means a sensor or actuator was checked against a suitable reference under documented conditions.

One level does not imply the next. In particular, an upload success message is not a runtime acceptance result.

## Start here

- [Getting started](getting-started.md)
- [Board matrix](hardware/boards.md)
- [Pinout](hardware/pinout.md)
- [Power and USB safety](hardware/power-usb.md)
- [Build and flash](firmware/build-flash.md)
- [Peripheral integration](peripherals/index.md)
- [Hardware validation matrix](hardware/validation.md)

Use the language selector in the header to switch between Chinese and English on the same page. Both language trees are built with strict fallback disabled, so an English page is never silently replaced by Chinese content.

## Safety boundary

Examples in this guide are intended for development and environmental trending. They are not certified life-safety, fire, gas-leak, or regulatory CO₂ instruments. Verify voltage, current, pin conflicts, and the exact module schematic before connecting hardware.

## Source policy

Board facts are checked against Seeed documentation; framework behavior is checked against the ESP-IDF version integrated by PlatformIO; peripheral facts are checked against component-vendor datasheets and official module schematics. Each research page records its evidence boundary and review date.
