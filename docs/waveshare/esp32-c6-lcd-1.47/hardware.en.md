# Hardware and pinout

## Onboard wiring

| Function | Signal | GPIO | Notes |
|---|---|---:|---|
| LCD | MOSI / SCLK | 6 / 7 | Shared with microSD |
| LCD | CS / D-C / RESET / BL | 14 / 15 / 21 / 22 | Backlight is active-high PWM |
| microSD | MISO / MOSI / SCLK / CS | 5 / 6 / 7 / 4 | SPI mode only |
| RGB LED | DIN | 8 | Addressable one-wire LED |
| USB | D− / D+ | 12 / 13 | Native USB; do not reuse casually |
| BOOT | button | 9 | Active low and a strapping pin |
| RESET | button | EN | Chip reset |

The table was cross-checked against the product page, two-page schematic and both vendor codebases. LCD and card share GPIO6/7, so one SPI bus owner must coordinate two chip selects. `SD_D1` and `SD_D2` are not connected; 4-bit SDMMC mode is unavailable.

## LCD

- Portrait resolution is 172×320. Vendor code uses RGB565, BGR order and display inversion.
- The ESP-IDF example sets a 12 MHz pixel clock. Higher third-party settings are not a board guarantee.
- Preserve the panel-specific initialization and offsets, then verify orientation, clipping and colors with a four-corner test image.
- GPIO22 controls an active-high MOSFET backlight. Keep normal brightness at 50% or below.

## Power

USB-C VBUS feeds `VCC_5V`; a `ME6217C33M5G` produces 3.3 V. Its advertised 800 mA maximum is not proof that the complete board can continuously supply that current under all thermal conditions. Measure radio, card and backlight peaks in the intended enclosure.

The header exposes 5 V and 3.3 V. ESP32-C6 GPIO uses 3.3 V logic and is not 5 V tolerant. Avoid back-powering between USB and an external 5 V source without a reviewed supply scheme.

## Allocation constraints

The schematic also breaks out GPIO0, 1, 2, 3, 10, 11, 18, 19, 20 and 23. Review the [ESP32-C6 datasheet](https://documentation.espressif.com/esp32-c6_datasheet_en.html) before allocation. GPIO8 is the RGB LED, GPIO9 is BOOT, GPIO12/13 are USB, and GPIO4–7/14/15/21/22 serve the display or card.

## Identify the exact product

The non-touch board uses ESP32-C6FH4, 4 MB Flash and ST7789. Specifications for the touch board—ESP32-C6FH8, 8 MB Flash, JD9853, AXS5106L and QMI8658—do not apply.
