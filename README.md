# XIAO ESP32 PlatformIO + ESP-IDF 工程

[![Firmware](https://github.com/ashllll/xiao-esp32-platformio-idf/actions/workflows/firmware.yml/badge.svg)](https://github.com/ashllll/xiao-esp32-platformio-idf/actions/workflows/firmware.yml)
[![Documentation](https://github.com/ashllll/xiao-esp32-platformio-idf/actions/workflows/docs.yml/badge.svg)](https://github.com/ashllll/xiao-esp32-platformio-idf/actions/workflows/docs.yml)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)

面向 Seeed Studio XIAO ESP32C3、ESP32S3 和 ESP32C6 的可复用项目基线。它把固件、板级引脚、外围设备资料、构建验证和可发布文档放在同一个仓库中，适合从小型原型持续演进为可复现、可维护的嵌入式项目。

- PlatformIO 管理 ESP-IDF、工具链、依赖、编译和烧录。
- MkDocs Material 管理硬件、固件、引脚及外围设备文档。
- GitHub Actions 验证全部板卡环境并构建文档站。
- 自动验收脚本验证真机启动，标准交付包携带版本、SHA-256 和烧录参数。

> [!IMPORTANT]
> 本仓库区分“编译通过”“真机通过”和“传感器指标有效”。构建成功不代表硬件已经验证；TVOC 算法推算出的 eCO₂ 也不等于直接测量的 CO₂。

在线手册：[ashllll.github.io/xiao-esp32-platformio-idf](https://ashllll.github.io/xiao-esp32-platformio-idf/)

English documentation: [ashllll.github.io/xiao-esp32-platformio-idf/en/](https://ashllll.github.io/xiao-esp32-platformio-idf/en/). The website provides a page-aware Chinese/English language selector, localized navigation and search, and strict translation-parity checks.

可选的完整本地资料库由 `XIAO_ESP32_REFERENCE_ROOT` 指定，未设置时默认为 `~/ESP32_资料整理`；本地资料库不存在时不影响固件或文档构建。工程文档的“资料索引”页记录了经过核对的官方入口和本地资料用法。

## 当前能力

| 范围 | 已提供内容 |
|---|---|
| 板卡 | XIAO ESP32C3、ESP32S3、ESP32C6 的独立 PlatformIO 环境和统一 D0–D10 符号 |
| 固件 | ESP-IDF 6.0.1、启动时芯片/Flash/PSRAM 自检、USB 日志和 LED 心跳 |
| 外设 | I²C、SPI、UART、GPIO、ADC、PWM，以及 ADS1115 和空气/环境传感器接入指南 |
| 文档 | MkDocs Material 中英双语站点、官方资料索引、外设页模板和严格链接/导航/翻译配对检查 |
| 交付 | 三板 CI 构建、可追溯 manifest、SHA-256 校验和、完整 Flash 参数 |
| 验收 | 串口 READY/heartbeat 协议；可只读验证，也可在显式授权后烧录验证 |

### 环境传感器与显示模块

| 器件/模块 | 实际能力 | 使用边界 |
|---|---|---|
| DFRobot Fermion ENS160（SEN0515） | AQI、TVOC、基于气体响应推算的 eCO₂ | eCO₂ 不是 NDIR/光声方式直接测量的 CO₂；要处理预热、首次启动和有效性状态 |
| Sensirion SGP41 / 通用 GY-SGP41 | 原始 VOC、NOx 信号；配合 Gas Index Algorithm 生成 VOC/NOx Index | 不输出 TVOC ppm、eCO₂ 或 CO₂；通用模块的稳压和电平转换能力必须按实物核对 |
| Sensirion SHT40 / Qwiic 模块 | 温度、相对湿度，可为气体传感器提供环境补偿 | 常见地址为 `0x44`，但具体后缀可能使用 `0x45`/`0x46`；Qwiic 总线按 3.3 V 使用 |
| 0.96 寸 128×64 I²C OLED | 单色图形显示 | 商品名不能确定 SSD1306/SH1106；先识别控制器、地址和模块电路 |
| TI OPT101 / 优信电子模块 | 模拟光功率信号 | 未经光谱与参考仪器标定不能直接称为 lux；输出必须符合 XIAO ADC 范围 |

详细接线、寄存器流程、预热条件和一手来源见[外围设备手册](https://ashllll.github.io/xiao-esp32-platformio-idf/peripherals/)。这些器件用于环境趋势与空气质量评估，不能替代生命安全、消防、燃气泄漏或合规 CO₂ 报警设备。

## 板卡支持矩阵

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

## 快速开始

克隆并安装固定版本的开发依赖：

```bash
git clone https://github.com/ashllll/xiao-esp32-platformio-idf.git
cd xiao-esp32-platformio-idf
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -r requirements-dev.txt -r requirements-docs.txt
```

选择与手上开发板一致的环境进行构建：

```bash
pio run -e xiao_esp32c3
```

把环境名替换为实际板卡。构建不会写入开发板。只有在明确需要覆盖目标设备固件时，才运行：

```bash
pio run -e xiao_esp32c3 -t upload
pio device monitor -b 115200
```

串口出现下面的稳定标志后，说明固件已确认芯片系列与 Flash 容量符合所选配置；S3 还会确认 8 MB Octal PSRAM 已初始化：

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

启动本地文档站：

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements-docs.txt
mkdocs serve
```

完整手册从 [`docs/index.md`](docs/index.md) 开始，涵盖环境搭建、板型/引脚、电气安全、固件配置和发布、总线与外围设备、无线连接、调试及资料维护。新增模块时复制 [`docs/peripherals/_template.md`](docs/peripherals/_template.md)，不要只把数据手册散放在仓库里。

默认环境在 `platformio.ini` 的 `default_envs` 中修改。

## 工程结构

```text
.
├── components/xiao_board/   # 板级自检、LED 心跳和统一板卡信息
├── include/xiao_pins.h      # XIAO D0–D10 到各板 GPIO 的符号映射
├── src/main.c               # 最小应用入口
├── docs/                    # MkDocs 手册、外设指南和来源核对记录
├── scripts/                 # 文档检查、真机验收和固件打包
├── tests/                   # 主机端回归测试
└── platformio.ini           # C3/S3/C6 构建环境
```

新增原生 ESP-IDF 外设驱动放入 `components/<device>/`。业务代码使用 `XIAO_I2C_SDA`、`XIAO_I2C_SCL` 等板级符号，不写死某一型号的裸 GPIO。

## 推荐工作流

1. 选定板型，在对应环境完成无硬件构建。
2. 按丝印 D0–D10 设计接线，并核对电压、上拉、电流峰值和引脚冲突。
3. 从外设模板创建资料页，优先引用芯片原厂数据表、模块厂商原理图和 ESP-IDF 官方 API。
4. 将驱动实现为独立 ESP-IDF 组件，先做设备 ID、地址探测和错误路径验证。
5. 运行三板构建、单元测试和严格文档构建。
6. 在明确设备和端口后进行真机验收；发布时生成带来源提交和校验值的固件包。

只使用某一块板开发时，也建议保留三环境构建：它能尽早发现裸 GPIO、芯片专属 API 或内存配置意外泄漏到共享代码的问题。

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

也可以一次运行项目级完整检查：

```bash
python3 ~/.agents/skills/manage-xiao-esp32-projects/scripts/validate_project.py . --build --docs
```

上面的全局 Skill 是可选的自动化入口，不是仓库构建依赖；没有安装 Skill 时，前一组标准命令仍可独立工作。

构建后可生成带校验值的交付包：

```bash
python3 scripts/package_firmware.py --environment xiao_esp32s3
```

Apple Silicon 上还应确认 `pio system info` 为 `darwin_arm64`，RISC-V/Xtensa 编译器宿主可执行文件为 Mach-O arm64。

## 本地资料与便携性

大体积 PDF、历史工程和厂商下载包不提交到本仓库。需要关联本地资料时使用：

```bash
export XIAO_ESP32_REFERENCE_ROOT="$HOME/ESP32_资料整理"
export XIAO_ESP32_PROJECT_ROOT="$HOME/code/esp/xiao_esp32"
```

两者都不是固件或文档构建的必需变量。公共文档应保留可访问的官方链接、检索日期和核对结论，不应写入个人绝对路径、Wi-Fi 凭据、Token 或私钥。

## 项目状态与贡献

参与修改前请阅读 [CONTRIBUTING.md](CONTRIBUTING.md)；安全问题按 [SECURITY.md](SECURITY.md) 报告。问题和改进建议可通过 [GitHub Issues](https://github.com/ashllll/xiao-esp32-platformio-idf/issues) 提交。
