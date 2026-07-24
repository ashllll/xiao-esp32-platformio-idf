# Waveshare ESP32-C6-LCD-1.47 Primary-Source Review

Retrieved: 2026-07-25 (Asia/Shanghai)

> This is the research basis for the board documentation. It covers the **non-touch ESP32-C6-LCD-1.47 / ESP32-C6-LCD-1.47-M**, not the similarly named `ESP32-C6-Touch-LCD-1.47`. Facts are based primarily on Waveshare design files and demos, Espressif documentation, and the supplied LCD module specification.

## 1. Key conclusions

- The SoC is `ESP32-C6FH4` with 4 MB in-package flash, not the 8 MB Touch model. Its HP and LP RISC-V cores run at up to 160 MHz and 20 MHz respectively.
- The 1.47-inch, 172 × 320, 262K-color TFT uses 4-wire SPI. Waveshare calls the controller ST7789; the LCD specification identifies module `LBS147TC-IF15` and driver `ST7789V3`.
- LCD and microSD share MOSI/SCLK on GPIO6/GPIO7 and have separate chip selects. Software must arbitrate the bus and keep each inactive device deselected.
- USB-C connects directly to ESP32-C6 USB Serial/JTAG; the schematic has no USB-UART bridge. Arduino `Serial` needs the appropriate USB CDC/HWCDC configuration.
- Waveshare explicitly says to keep backlight brightness at **50% or lower** and avoid extended full-brightness use because heating may cause a dark shadow.
- The legacy Wiki contains a copied instruction to select `ESP32S3 Dev Module`. That conflicts with the schematic and SoC and must not be followed.

## 2. Identity, processor, and radio

| Item | Verified value | Primary evidence |
|---|---|---|
| Products | ESP32-C6-LCD-1.47, SKU 28563; `-M` with headers, SKU 30381 | [Waveshare documentation](https://docs.waveshare.com/ESP32-C6-LCD-1.47) |
| SoC | ESP32-C6FH4, QFN32, 4 MB flash in package | [Schematic](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf), [Espressif datasheet v1.5](https://documentation.espressif.com/esp32-c6_datasheet_en.pdf) |
| CPU/memory | HP RISC-V 160 MHz, LP RISC-V 20 MHz; 320 KB ROM, 512 KB HP SRAM, 16 KB LP SRAM | [Espressif datasheet](https://documentation.espressif.com/esp32-c6_datasheet_en.pdf) |
| Radio | 2.4 GHz Wi-Fi 6 (802.11ax, b/g/n compatible), Bluetooth LE, IEEE 802.15.4 (Thread 1.3 / Zigbee 3.0) | [Espressif datasheet](https://documentation.espressif.com/esp32-c6_datasheet_en.pdf) |
| Antenna | Onboard ceramic antenna and matching network | [Schematic](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf) |

Waveshare says “Bluetooth 5”; the current Espressif datasheet says “Bluetooth LE 5.3 certified.” Documentation should say Bluetooth LE and must not imply Classic Bluetooth support.

## 3. Onboard hardware and pins

| Function | GPIO | Notes |
|---|---:|---|
| LCD MOSI / DIN | 6 | Shared with microSD |
| LCD SCLK | 7 | Shared with microSD |
| LCD CS / D-C / RESET | 14 / 15 / 21 | CS and reset active low |
| LCD backlight PWM | 22 | Drives SI2302 N-MOS through 1 kΩ |
| WS2812B data | 8 | One onboard RGB LED |
| microSD MISO / MOSI / SCLK / CS | 5 / 6 / 7 / 4 | SPI mode; D1 and D2 not connected |
| BOOT | 9 | Button pulls low; strapping pin |
| RESET | CHIP_PU | Not an ordinary GPIO |
| USB D- / D+ | chip USB function on 12 / 13 | Native USB Serial/JTAG |

Sources: [Waveshare interface table](https://docs.waveshare.com/ESP32-C6-LCD-1.47), [schematic](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf), and [Espressif datasheet](https://documentation.espressif.com/esp32-c6_datasheet_en.pdf).

The board also exposes power, ground, GPIO0/1/2/3/18/19/20/23, and UART signals. Physical placement must be checked against the official pinout/schematic. GPIO4–8, 14, 15, 21, and 22 already serve onboard devices; GPIO9 is BOOT; GPIO12/13 serve USB. “Most GPIOs exposed” does not mean conflict-free. The microSD wiring is SPI-only, not 4-bit SDMMC.

## 4. Power and electrical boundaries

- USB VBUS becomes `VCC_5V`; a `ME6217C33M5G` LDO generates 3.3 V. Waveshare labels it 800 mA maximum, but that is not a guaranteed continuous board-level current under all thermal conditions.
- Both USB-C CC pins have 5.1 kΩ pull-downs. There is no USB-UART bridge in the schematic.
- The LCD module specifies VDD 2.5–3.3 V, interface voltage 1.65–3.3 V, and a two-LED backlight at typical 3.0 V/40 mA (60 mA listed maximum). Module operating/storage ranges are -20–70 °C and -30–80 °C.
- No official board derating curve or external-load budget was found. Budget ESP32 radio peaks, LCD/backlight, microSD transients, RGB LED, dropout, and thermal dissipation.
- No power OR-ing or backfeed specification was found. Do not simultaneously back-power the USB, 5 V, and 3.3 V rails.

Sources: [schematic](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf) and [LCD specification](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/1.47_LCD_Manual.pdf).

## 5. LCD parameters

| Parameter | Value |
|---|---|
| Module / revision | LBS147TC-IF15, A |
| Panel | Normally-black TFT, all-view |
| Resolution / colors | 172(H) RGB × 320(V), 262K |
| Interface / driver | 4-line SPI, ST7789V3 |
| Active area | 17.3892 × 32.352 mm |
| Module size | 19.39 × 36.28 × 1.46 mm |
| Pixel pitch | 0.0337(H) × 0.1011(V) mm |
| Typical luminance | 350 cd/m² |

These are LCD-module dimensions, not board dimensions. Board dimensions are only available graphically and were not reliably extractable, so no value is guessed here. The LCD table says ST7789V3, while one mechanical drawing label says `STV7789V3/GC9307N`; Waveshare pages and both demos use the ST7789 family. Confirm the physical batch before production. Use the official initialization sequence, orientation, offsets, and color order as the board baseline.

## 6. Development environments and examples

### Arduino

- Required Arduino core: `esp32 by Espressif Systems >= 3.0.0`.
- Reproducible supplied libraries: LVGL 8.3.10 and PNGdec 1.0.2. An LVGL 8 display port is not automatically compatible with LVGL 9.
- `LVGL_Arduino` exercises flash, LCD, LVGL, microSD, and wireless scan.
- `LVGL_Image` (called `LCD_Image` on the old Wiki) reads PNG files no larger than 172 × 320 from microSD.
- Inspection of the official ZIP confirms 172 × 320, LCD CS/DC/RST 14/15/21, and SD CS 4.
- Select an ESP32-C6-compatible generic target, not the erroneous `ESP32S3 Dev Module`.

See [Working with Arduino](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Development-Environment-Setup-Arduino).

### ESP-IDF

- Waveshare requires ESP-IDF 5.3.1 or later. Current screenshots use 5.5.2, but the version should match the downloaded example.
- `ESP32-C6-LCD-1.47-Test` contains ST7789T, LVGL, microSD, wireless, and RGB support; target is `esp32c6`.
- It tests flash, wireless, RGB, SD, LCD, 50% backlight, LVGL, and CPU usage.
- Normal CLI flow is `idf.py set-target esp32c6`, `idf.py build`, then `idf.py -p <PORT> flash monitor`.
- In this PlatformIO + ESP-IDF repository, port the IDF components/CMake/Kconfig dependencies. Do not mix `.ino`, Arduino `SPI`, or Arduino `SD` APIs unless Arduino-as-component is intentionally introduced.

See [Working with ESP-IDF](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Development-Environment-Setup-ESP-IDF) and the [official ESP-IDF ESP32-C6 guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/).

## 7. Flashing, serial, and recovery

1. Use a data-capable USB-C cable and select ESP32-C6 plus the detected port.
2. If flashing/connection fails, hold BOOT, press and release RESET, then release BOOT and retry.
3. If ESP-IDF stops at `waiting for download...` after flashing, press RESET or power-cycle.
4. For missing Arduino `Serial` output, verify USB CDC On Boot/HWCDC settings.
5. The ZIP contains `Firmware/ESP32-C6-LCD-1.47-Test.bin`; use Waveshare's documented address/flash procedure rather than guessing an address.
6. A successful build/upload is not hardware acceptance. Verify LCD colors/orientation/edges, backlight PWM, SD read/write, RGB, radio scan, reset, and repeated flashing.

Source: [Waveshare FAQ](https://docs.waveshare.com/ESP32-C6-LCD-1.47/FAQ).

## 8. Safety and integration notes

- Keep brightness at or below 50%; do not ship full brightness as the default.
- Do not remove the display when soldering the non-header board; tilt the iron. Do not pry apart the bonded display/back cover.
- Do not externally force GPIO9 to the wrong level during reset.
- Reusing native USB pins sacrifices the normal download/log path and can create electrical contention.
- Arbitrate the shared LCD/SD SPI bus and chip-select states.
- The board works without a TF card, but applications must handle card absence cleanly.
- This non-touch model has no touch controller, IMU, or battery charger.

## 9. Conflicts and unresolved items

| Topic | Resolution |
|---|---|
| User-supplied Chinese Wiki | The retrieved `waveshare.net` page is effectively an empty legacy shell; the official new documentation platform and international Wiki were used to complete it. |
| Arduino target | Legacy Wiki says ESP32S3; rejected as a template-copy error. Use ESP32-C6. |
| LCD controller | ST7789 vs ST7789V3 vs a drawing's STV7789V3/GC9307N label; current official code uses the ST7789 family, but batch substitution remains unconfirmed. |
| Bluetooth wording | Waveshare says Bluetooth 5; Espressif says Bluetooth LE 5.3 certified. Do not claim Classic Bluetooth. |
| Board dimensions | Only graphical/CAD sources found; no unverified numeric transcription. |
| Maximum SPI clock | No complete, reliably extracted timing guarantee; do not turn a demo clock into a device rating. |
| External power budget | No board-level thermal/current/backfeed specification. |
| Exact tool versions | Only minimum versions and a ZIP snapshot are given; record actual core/IDF versions and ZIP hash when reproducing. |
| LCD-controller original manufacturer | The supplied module PDF names Limito Technology, but no independently verifiable original ST7789V3 controller datasheet was linked. |

## 10. Official resource index

All links retrieved 2026-07-25:

- [Chinese Wiki supplied by the user](http://www.waveshare.net/wiki/ESP32-C6-LCD-1.47)
- [Waveshare documentation](https://docs.waveshare.com/ESP32-C6-LCD-1.47)
- [Resources](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Resources-And-Documents)
- [Arduino guide](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Development-Environment-Setup-Arduino)
- [ESP-IDF guide](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Development-Environment-Setup-ESP-IDF)
- [FAQ](https://docs.waveshare.com/ESP32-C6-LCD-1.47/FAQ)
- [Schematic PDF](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf)
- [3D drawing RAR](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47-3D_Drawing.rar)
- [Official demo ZIP](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47-Demo.zip)
- [LBS147TC-IF15 LCD specification](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/1.47_LCD_Manual.pdf)
- [Espressif ESP32-C6 datasheet v1.5](https://documentation.espressif.com/esp32-c6_datasheet_en.pdf)
- [ESP-IDF ESP32-C6 guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/)
- [Arduino-ESP32 documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [Waveshare ESP32-display-support](https://github.com/waveshareteam/ESP32-display-support)
- [LVGL 8.3 documentation](https://docs.lvgl.io/8.3/)

Community examples are inspiration, not hardware authority: [VolosR](https://github.com/VolosR/WaveShareC6lvglexample), [LVGL9 crypto monitor](https://github.com/thelastoutpostworkshop/ESP32-C6-LCD-1.47_LVGL9_Crypto_Monitor), and [video player](https://github.com/thelastoutpostworkshop/ESP32-C6-LCD-1.47_video_player).

## 11. Acceptance checklist before publication

- [ ] Record the official ZIP SHA-256 and the versions inside it.
- [ ] Build for C6 in this repository's actual PlatformIO ESP-IDF environment.
- [ ] Confirm physical silkscreen, SoC marking, display batch, and board dimensions.
- [ ] Measure heating at 50% brightness and confirm PWM polarity/duty semantics.
- [ ] Test red/green/blue/white/black, corners, orientation, and full display bounds.
- [ ] Test SD absent/present/read/write and LCD/SD bus switching.
- [ ] Repeat reset, BOOT download mode, USB logs, and factory recovery.
- [ ] Validate Wi-Fi, BLE, and 802.15.4 separately for the shipped firmware and regulatory target.
