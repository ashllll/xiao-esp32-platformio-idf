# iKuai monitor example firmware

This example turns the ESP32-C6-LCD-1.47 into a 320×172 UniFi-LCM-style
network status display with WAN state, download/upload rates, client count,
ping, and an approximately ten-second three-color trend.

## Downloads

- [v1.0.0 release](https://github.com/ashllll/xiao-esp32-platformio-idf/releases/tag/esp32-c6-lcd-monitor-v1.0.0)
- [privacy-safe source](https://github.com/ashllll/xiao-esp32-platformio-idf/tree/main/examples/waveshare_esp32_c6_lcd_1_47_monitor)

The release contains a merged image, separate bootloader/partition/application
images, flash arguments, a manifest, SHA-256 checksums, and a README.

The prebuilt image runs in **offline demo mode** with synthetic values. It does
not connect to a network and contains no Wi-Fi password, router address, API
token, or iKuai certificate.

For fuzzy edges, gray halos, or scaling blur, follow [Crisp text and blur
troubleshooting](font-rendering.md). It covers native resolution, font bpp,
label coordinates, RGB565 byte order, and a build-tested LVGL helper.

## Flash the prebuilt demo

Confirm that the board is the non-touch ST7789 model. Do not flash this image to
the JD9853 touch model. Flashing overwrites the installed application and
configuration.

```bash
python3 -m esptool --chip esp32c6 \
  --port /dev/cu.usbmodemXXXX \
  write_flash 0x0 merged-flash.bin
```

The package uses only the first 4 MB so it covers the documented 4 MB and 8 MB
non-touch batches. Still inspect the actual chip with `esptool flash_id` first.
The driver clamps backlight brightness to 40%.

## Enable live iKuai data

```bash
./scripts/fetch_lvgl.sh
cp src/config.example.h src/config.h
cp src/ikuai_cert.example.h src/ikuai_cert.h
```

Edit the ignored `src/config.h`, set `APP_DEMO_MODE` to `0`, and fill in
`APP_WIFI_SSID`, `APP_WIFI_PASS`, `IKUAI_HOST`, and `IKUAI_TOKEN`. Convert the
certificate presented by the iKuai HTTPS endpoint and build:

```bash
python3 scripts/pem_to_header.py ikuai.pem src/ikuai_cert.h
pio run -e esp32-c6-lcd
```

Never commit `src/config.h`, `src/ikuai_cert.h`, real tokens, or personalized
firmware binaries.

## Validation boundary

The live-configured version was verified on an ESP32-C6FH8 board for LCD,
Wi-Fi, IP acquisition, and sustained curve updates at approximately 26–27 FPS.
The public v1.0.0 offline image passed a clean build and credential scan but was
not written to hardware again before publication; build evidence is not a
separate physical acceptance result.
