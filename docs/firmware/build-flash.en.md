# Build and flash

## Build

```bash
pio run -e xiao_esp32c3
pio run -e xiao_esp32s3
pio run -e xiao_esp32c6
```

Artifacts include `firmware.bin`, `bootloader.bin`, the partition table, and `flash_args` under `.pio/build/<environment>/`. A successful build does not write hardware.

## Find the serial port

```bash
pio device list
```

Compare before and after connection. Prefer `/dev/cu.*` on macOS and close other serial monitors. If no port appears, check the data cable, hub, power, and download mode.

## Flash and monitor

```bash
pio run -e xiao_esp32c3 -t upload
pio device monitor -b 115200
```

Upload success only proves that bytes were written. Runtime acceptance additionally requires the correct chip and memory report, `XIAO_RUNTIME_READY version=1`, and ten consecutive heartbeats.

```bash
python3 scripts/verify_hardware.py \
  --environment xiao_esp32s3 --port auto --heartbeats 10
```

Add `--flash` only when replacing the existing device firmware is intended.

## Package a delivery

```bash
python3 scripts/package_firmware.py --environment xiao_esp32s3
```

`dist/<environment>/` contains the application, bootloader, partition table, portable flash arguments, `manifest.json`, and `SHA256SUMS`. Packaging rejects an application above 768 KiB for the template's 1 MiB app partition.

## Recovery

If automatic download fails, hold BOOT, tap RESET, release BOOT, and retry. An erase destroys NVS, provisioning, and application data:

```bash
pio run -e xiao_esp32c3 -t erase
```

Confirm the exact environment and port before erasing.

## Apple Silicon check

`pio system info` should report `darwin_arm64`, while the installed Xtensa and RISC-V compiler executables should be Mach-O arm64 host binaries. Their names describe the firmware target, not the Mac architecture.
