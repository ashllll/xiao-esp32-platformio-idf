# Getting started

## 1. Install the pinned tools

Python 3, Git, and a working virtual-environment module are required.

```bash
git clone https://github.com/ashllll/xiao-esp32-platformio-idf.git
cd xiao-esp32-platformio-idf
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -r requirements-dev.txt -r requirements-docs.txt
```

The project pins PlatformIO Core 6.1.19, Espressif 32 Platform 7.0.1, and its officially integrated ESP-IDF 6.0.1. Do not override only `framework-espidf` with a newer tag: the build scripts, tools, and framework must be validated as one set.

## 2. Select the physical board

| Board | Environment | Flash / PSRAM |
|---|---|---|
| XIAO ESP32C3 | `xiao_esp32c3` | 4 MB / none onboard |
| XIAO ESP32S3 | `xiao_esp32s3` | 8 MB / Octal PSRAM |
| XIAO ESP32C6 | `xiao_esp32c6` | 4 MB / none onboard |

Confirm Sense, Plus, or other expansion-board variants separately; they do not share every resource mapping.

## 3. Build without writing hardware

```bash
pio run -e xiao_esp32c3
```

Replace the environment name with the board you own. A successful build proves configuration and linking only.

## 4. Inspect the generated configuration

```bash
rg 'CONFIG_ESPTOOLPY_FLASHSIZE|CONFIG_SPIRAM' \
  .pio/build/xiao_esp32s3/config/sdkconfig.h
```

Expected results are 4 MB Flash for C3/C6, and 8 MB Flash plus `CONFIG_SPIRAM` and `CONFIG_SPIRAM_MODE_OCT` for S3.

## 5. Flash only when you intend to replace the device firmware

```bash
pio run -e xiao_esp32c3 -t upload
pio device monitor -b 115200
```

The accepted runtime must print `XIAO_RUNTIME_READY version=1` and at least ten monotonically increasing heartbeat lines. If chip family, Flash, or S3 PSRAM differs from the selected profile, the firmware stops before READY.

Use the guarded verifier for repeatable acceptance:

```bash
python3 scripts/verify_hardware.py \
  --environment xiao_esp32s3 --port auto --heartbeats 10
```

The command above reads the current firmware. Add `--flash` only with explicit intent to overwrite it. When multiple Espressif serial devices are connected, the tool refuses to guess.

## 6. Build the documentation site

```bash
python3 scripts/validate_docs.py
mkdocs build --strict
mkdocs serve
```

The root site is Chinese and the complete English site is under `/en/`. The header language selector stays on the equivalent page.

## Next steps

Read the [pinout](hardware/pinout.md), [power and USB safety](hardware/power-usb.md), and [development workflow](workflow.md) before wiring a peripheral.
