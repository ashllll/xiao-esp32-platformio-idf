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

## Reproducibility

The vendor pages can change and the ZIP has no stable version tag. Record retrieval date, ZIP SHA-256, ESP-IDF/Arduino/LVGL versions, board SKU, Flash ID and logs. Avoid “latest” as a version.
