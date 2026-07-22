# Debugging

Start by identifying which evidence layer failed: dependency installation, compile/link, upload, boot/runtime, bus communication, or measurement validity.

## Build failures

Run one environment with verbose output and confirm the pinned platform/framework:

```bash
pio run -e xiao_esp32c3 -v
pio system info
```

Check component `REQUIRES`, target guards, generated `sdkconfig.h`, and whether an example accidentally uses Arduino APIs in this ESP-IDF project.

## No serial port

Compare `pio device list` before and after connection. Try a known data cable, direct port, and correct BOOT/RESET sequence. Close serial monitors that already own the port. Multiple Espressif devices require an explicit port.

## Upload succeeds but READY is absent

Capture the complete boot log. Check reset reason, brownout, panic, chip mismatch, Flash size, and S3 PSRAM initialization. Do not remove the runtime check to hide the mismatch.

## I²C/SPI/UART failures

Verify supply, common ground, logic levels, actual module schematic, pin symbols, address/chip-select, bus mode/rate, pull-ups, and expansion conflicts. Use an address scan or low-rate deterministic transfer only as a diagnostic; it does not validate measurement quality.

## Intermittent resets

Measure supply at the board during radio or load peaks. Inspect brownout logs, cable/hub voltage drop, decoupling, stack size, heap use, watchdog events, and unsafe cross-task ownership. Reproduce with one variable changed at a time.

## Evidence capture

Record board revision, environment, source commit, command, port, power setup, attached peripherals, complete error text, and whether hardware was written. A screenshot without these identifiers is weak evidence.

For guarded runtime capture, use `scripts/verify_hardware.py`; `--flash` remains an explicit overwrite operation.
