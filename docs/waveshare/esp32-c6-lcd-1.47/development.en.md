# Development and vendor examples

## Choose the correct project

| Path | Vendor baseline | Bundle | Repository boundary |
|---|---|---|---|
| Arduino | Espressif Arduino core ≥ 3.0.0 | `LVGL_Arduino`, `LVGL_Image`, LVGL 8.3.10, PNGdec 1.0.2 | Reference only; do not mix Arduino APIs into pure ESP-IDF |
| Native ESP-IDF | ESP-IDF ≥ 5.3.1; bundle declares 5.3 | LVGL, LCD, card, RGB and Wi-Fi test | Review APIs before porting to ESP-IDF 6.0.1 |
| This PlatformIO repo | Espressif32 7.0.1 / ESP-IDF 6.0.1 | XIAO environments only | A dedicated board environment is required before building or flashing |

The vendor ZIP is about 60 MB and expands to roughly 268 MB because it vendors libraries, images and firmware. Do not commit that mirror; retain the URL, version and optionally a SHA-256, then import only reviewed minimal code.

## Native ESP-IDF

```bash
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/cu.usbmodemXXXX flash monitor
```

Flashing overwrites installed firmware. Preserve a recovery path first. To enter ROM download mode, hold BOOT, tap RESET, release RESET, then release BOOT.

Create one `SPI2_HOST` bus with SCLK 7, MOSI 6 and MISO 5. Add the LCD (CS 14, D/C 15, vendor example at 12 MHz) and card (CS 4) as separate devices. Keep inactive CS lines high. Start the backlight at zero and raise it only after the first valid frame, stopping at 50%.

Treat a missing card as a recoverable condition rather than rebooting the board. Serialize LVGL and filesystem activity through the SPI driver.

## Arduino

The vendor pins LVGL 8.3.10 and PNGdec 1.0.2 and warns that LVGL 8 and 9 drivers are not interchangeable. Select a generic `ESP32C6 Dev Module`; the old Wiki's ESP32S3 selection text is a template error.

`LVGL_Arduino` reports Flash, card and wireless scan data. `LVGL_Image` rotates PNG files from the card root, with images no larger than 172×320. Pixel dimensions do not remove file-size, color-format or decoder-memory constraints.

## LVGL and memory

The ESP-IDF example uses LVGL 8 and a `172×20` partial buffer. A LVGL 9 move requires the official migration steps for display, flush, tick and handler APIs. Use one UI owner task, validate DMA-capable allocations, and record minimum free heap plus the largest block while LCD, card and Wi-Fi are active.

## Factory test image

`Firmware/ESP32-C6-LCD-1.47-Test.bin` is useful for recovery and board checks. Do not guess its flash address; use the vendor flashing instructions or bundled flash arguments. Restoring it overwrites the application and settings.
