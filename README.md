# XIAO ESP32 PlatformIO + ESP-IDF 工程

[![Firmware](https://github.com/ashllll/xiao-esp32-platformio-idf/actions/workflows/firmware.yml/badge.svg)](https://github.com/ashllll/xiao-esp32-platformio-idf/actions/workflows/firmware.yml)
[![Documentation](https://github.com/ashllll/xiao-esp32-platformio-idf/actions/workflows/docs.yml/badge.svg)](https://github.com/ashllll/xiao-esp32-platformio-idf/actions/workflows/docs.yml)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)

面向 Seeed Studio XIAO ESP32C3、ESP32S3 和 ESP32C6 的个人项目模板：

- PlatformIO 管理 ESP-IDF、工具链、依赖、编译和烧录。
- MkDocs Material 管理硬件、固件、引脚及外围设备文档。
- GitHub Actions 验证全部板卡环境并构建文档站。
- 自动验收脚本验证真机启动，标准交付包携带版本、SHA-256 和烧录参数。

在线手册：[ashllll.github.io/xiao-esp32-platformio-idf](https://ashllll.github.io/xiao-esp32-platformio-idf/)

可选的完整本地资料库由 `XIAO_ESP32_REFERENCE_ROOT` 指定，未设置时默认为 `~/ESP32_资料整理`；本地资料库不存在时不影响固件或文档构建。工程文档的“资料索引”页记录了经过核对的官方入口和本地资料用法。

## 支持矩阵

| 环境 | 板卡 | Flash/PSRAM | 当前证据 |
|---|---|---|---|
| `xiao_esp32c3` | Seeed XIAO ESP32C3 | 4 MB / 无板载 PSRAM | 构建与交付包通过；真机未记录 |
| `xiao_esp32s3` | Seeed XIAO ESP32S3 | 8 MB / Octal PSRAM | 构建与交付包通过；有历史只读证据，当前设备待重验 |
| `xiao_esp32c6` | Seeed XIAO ESP32C6 | 4 MB / 无板载 PSRAM | 构建与交付包通过；真机未记录 |

构建成功只证明工具链、API 和配置能够生成固件，不等同于真实硬件通过。S3 曾有未绑定 Git 提交的只读验收记录，但本轮连接设备运行的是其他项目固件，不能作为本工程当前真机证据；准确边界见[真机验证矩阵](docs/hardware/validation.md)。

## 环境要求

- macOS 或 Linux；本项目在 Apple Silicon `darwin_arm64` 上验证。
- Python 3、Git，以及可创建虚拟环境的 Python 安装。
- 固定依赖：PlatformIO Core 6.1.19、Espressif32 Platform 7.0.1、ESP-IDF 6.0.1、MkDocs 1.6.1、Material 9.7.7。

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -r requirements-dev.txt -r requirements-docs.txt
```

## 快速开始

```bash
pio run -e xiao_esp32c3
```

把环境名替换为实际板卡。构建不会写入开发板。只有在明确需要覆盖目标设备固件时，才运行：

```bash
pio run -e xiao_esp32c3 -t upload
pio device monitor -b 115200
```

串口出现下面的稳定标志后，说明固件已确认
芯片系列与 Flash 容量符合所选配置；S3 还会确认 8 MB Octal PSRAM 已初始化：

```text
XIAO_RUNTIME_READY version=1 board=Seeed Studio XIAO ESP32C3
heartbeat count=1 uptime_ms=...
```

如果自检不通过，组件会输出实际值和期望值并中止，不会用错误硬件配置继续运行。该检查验证芯片和内存基线，不用于鉴别板卡厂商或具体扩展板变体。

自动验收当前固件，或显式授权烧录后验收：

```bash
python3 scripts/verify_hardware.py --environment xiao_esp32s3 --port auto
python3 scripts/verify_hardware.py --environment xiao_esp32s3 --port auto --flash --json-output
```

脚本在出现多个 Espressif 串口时拒绝猜测。D6/D7 留给业务 UART；应用日志通过 USB Serial/JTAG 输出，ROM 启动阶段仍可能短暂使用 UART0。

启动文档站：

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements-docs.txt
mkdocs serve
```

完整手册从 [`docs/index.md`](docs/index.md) 开始，涵盖环境搭建、板型/引脚、电气安全、固件配置和发布、I²C/SPI/UART/GPIO/ADC/PWM、ADS1115 完整实例、无线连接、调试与资料维护。

默认环境在 `platformio.ini` 的 `default_envs` 中修改。

## 工程结构

- `src/main.c`：应用入口，演示 `xiao_board` 组件（LED 心跳 + 环境自检日志）。
- `components/xiao_board/`：板级支持组件；新增外设驱动放入 `components/<device>/`。
- `include/xiao_pins.h`：XIAO 丝印 D0–D10 到各板 GPIO 的符号映射，业务代码不写裸 GPIO。
- `docs/`：文档站源码；新增外设页从 `docs/peripherals/_template.md` 复制。

版本与变更记录见 `CHANGELOG.md`；许可证为 Apache-2.0（`LICENSE`）。

当前源码版本为 0.3.0，但仓库尚未创建对应 Git tag 或 GitHub Release。Actions artifact 适合构建验证，不是长期发布渠道；正式交付应按[固件发布](docs/firmware/release.md)创建不可变版本和完整三板交付包。

## 验证

```bash
python3 scripts/validate_docs.py
pio run -e xiao_esp32c3
pio run -e xiao_esp32s3
pio run -e xiao_esp32c6
.venv/bin/mkdocs build --strict
python3 -m unittest discover -s tests -v
```

构建后可生成带校验值的交付包：

```bash
python3 scripts/package_firmware.py --environment xiao_esp32s3
```

Apple Silicon 上还应确认 `pio system info` 为 `darwin_arm64`，RISC-V/Xtensa 编译器宿主可执行文件为 Mach-O arm64。

参与修改前请阅读 [CONTRIBUTING.md](CONTRIBUTING.md)；安全问题按 [SECURITY.md](SECURITY.md) 报告。
