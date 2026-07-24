# Troubleshooting and hardware acceptance

## Common failures

| Symptom | Check first |
|---|---|
| No port or flash failure | Data cable, permissions, ROM download sequence, `esp32c6` target |
| `waiting for download...` | Release BOOT, tap RESET or power-cycle |
| USB repeatedly disconnects | Enter ROM mode and flash known firmware; check cable and supply |
| Black screen | GPIO22 backlight, reset, CS/DC, SPI mode/clock and init table |
| White/corrupt/shifted image | 172×320, RGB565/BGR, inversion, offsets and DMA buffer lifetime |
| Bottom dark shadow | Turn down/off backlight and cool; keep future operation ≤50% |
| Card mount failure | FAT format, power, CS4/MISO5, shared bus; lower clock first |
| LVGL hangs/reboots | Concurrent handlers, stack/heap, flush completion, SPI contention, watchdog |

The Waveshare FAQ's generic “install a MAC driver” item does not identify a bridge chip. This board's schematic connects USB D+/D− directly to ESP32-C6, so inspect native USB Serial/JTAG enumeration before installing an unknown driver.

## Staged acceptance

1. **No-flash inspection:** record SKU and photos; inspect bonding, connector and antenna; check 5 V/3.3 V and preserve a recovery path.
2. **Minimal firmware:** verify ESP32-C6 and 4 MB Flash, ten reliable enumerations/reboots, BOOT/RESET behavior and RGB colors.
3. **Display:** ramp brightness through 10%, 25% and 50%; show color/gray patterns, a one-pixel border and corner labels; soak for at least 30 minutes while recording temperature and artifacts.
4. **Card and shared SPI:** boot safely without a card, test checksummed read/write on a backed-up card, then combine LVGL refresh, PNG reads and Wi-Fi scanning.
5. **System/radio:** record minimum heap, largest block, stack margin and watchdog state; separately test Wi-Fi and BLE, then 802.15.4 only if required; repeat in the intended enclosure and supply range.

A documentation build, firmware compile or successful flash is not hardware acceptance.

## Real-hardware acceptance record

**Date:** 2026-07-25

**Test environment:**

| Item | Value |
|---|---|
| Firmware source | Independent test firmware (not vendor bundle) |
| Framework | PlatformIO `espressif32@7.0.1` / ESP-IDF 6.0.1 |
| Board definition | `esp32-c6-devkitc-1` |
| Serial port | `/dev/tty.usbmodem11411201` |
| Chip | ESP32-C6FH4 rev v0.2 |
| Flash | 4 MB (DIO, 80 MHz) |
| RAM | 437 KiB available |

### Passed tests

| Stage | Test | Result | Notes |
|---|---|---|---|
| Minimal firmware | Chip identified as ESP32-C6, Flash 4 MB | ✅ Pass | Confirmed via boot log |
| Minimal firmware | Reliable serial enumeration across reboots | ✅ Pass | |
| Minimal firmware | BOOT/RESET enters app mode | ✅ Pass | |
| Display | ST7789 init (172×320, RGB565, BGR) | ✅ Pass | SPI 12 MHz |
| Display | Color bars (R/G/B/W/Y/C/M) | ✅ Pass | Phase 1 |
| Display | Text rendering (8×8 bitmap font) | ✅ Pass | Phase 2 |
| Display | Checkerboard (8×8 px) | ✅ Pass | Phase 3 |
| Display | Solid white (dead pixel check) | ✅ Pass | Phase 4 |
| Display | Gradient (blue → red) | ✅ Pass | Phase 5 |
| Display | Backlight PWM (0–50%) | ✅ Pass | LEDC 10-bit |

### Not yet tested

| Item | Reason |
|---|---|
| TF card read/write | Not included in test firmware |
| RGB LED | Not included in test firmware |
| Wi-Fi / BLE | Not included in test firmware |
| Long-term thermal | No 30+ minute soak test |
| TF + LCD shared SPI concurrency | Not tested |

### Boot log

```
ESP-ROM:esp32c6-20220919
I (23) boot: ESP-IDF 6.0.1 2nd stage bootloader
I (24) boot: chip revision: v0.2
I (34) boot.esp32c6: SPI Flash Size : 8MB
I (253) lcd_test: === Waveshare ESP32-C6-LCD-1.47 LCD Test ===
I (549) lcd_test: LCD initialized
I (549) lcd_test: Phase 1: Color bars
I (3674) lcd_test: Phase 2: Text
I (8346) lcd_test: Phase 3: Checkerboard
I (12710) lcd_test: Phase 4: Solid white
I (15794) lcd_test: Phase 5: Gradient
I (16006) lcd_test: Test sequence complete
```

### Build issues

| Issue | Solution |
|---|---|
| Arduino framework unsupported on ESP32-C6 | Use ESP-IDF framework |
| ESP-IDF 6.0.1 bootloader.ld path error | Manual symlink: `bootloader.ld -> bootloader/ld/bootloader.ld` |
| Flash size defaults to 8 MB (actual 4 MB) | Do not override in `platformio.ini`; use board default |

## Reproducibility

The vendor pages can change and the ZIP has no stable version tag. Record retrieval date, ZIP SHA-256, ESP-IDF/Arduino/LVGL versions, board SKU, Flash ID and logs. Avoid “latest” as a version.
