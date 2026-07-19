# 0.96 英寸 I²C 128×64 OLED 一手资料核对

检索日期：2026-07-19

“0.96 英寸、I²C、128×64 OLED”只是外观与接口描述，不能唯一确定控制器、模块供电、针序或复位电路。市售模块常见 SSD1306，也可能使用 SH1106 或兼容芯片。本文先给出可验证的芯片级事实，再列出实物识别和 ESP-IDF 接入边界。

## 结论摘要

- SSD1306 与 SH1106 都可能使用 `0x3C`/`0x3D`，I²C 扫描不能区分两者。必须结合卖家 BOM、芯片/排线标记、初始化效果和显存列偏移识别。
- SSD1306 有 128×64 GDDRAM，完整 framebuffer 为 1024 字节；SH1106 内部是 132×64，常见 128×64 面板需要列偏移，不能把两者当作完全相同的初始化与刷新协议。
- 两类控制器的 I²C Fast-mode 上限均为 400 kHz。共享总线时应按最慢设备、总线电容和并联上拉决定实际频率。
- Solomon Systech 给出的 `VDD=1.65–3.3 V` 等是 SSD1306 裸控制器电源轨，不是四针模块的 VIN 范围。未知模块不能自动按“3.3–5 V”供电。
- 4 针板通常没有外露 RESET；这只说明模块可能有 RC/电源复位，不能推断所有板的启动时序都相同。

## SSD1306 已证实能力

Solomon Systech 当前产品目录将 SSD1306 列为量产中的 128×64 单色 OLED 驱动器，带 charge pump，支持 I²C、SPI 和并行接口，并含 128×64 显示 SRAM。[SSD1306 产品页](https://www.solomon-systech.com/zh-hans/product/SSD1306)、[OLED 产品目录](https://www.solomon-systech.com/product-category/oled-display)

下列协议细节来自 Solomon Systech 编写的 [SSD1306 Datasheet Rev 1.1](https://files.waveshare.com/upload/a/af/SSD1306-Revision_1.1.pdf)（2008-04）。该 PDF 由 Waveshare 镜像托管，因为原厂当前页面只提供索取数据表入口；使用时应保留版本信息。

| 项目 | SSD1306 |
|---|---|
| 可驱动分辨率 | 128 segments × 64 commons |
| GDDRAM | 128 × 64 bits，即 1024 bytes、8 pages |
| 7 位 I²C 地址 | `0x3C` 或 `0x3D`，由 SA0 决定 |
| I²C 上限 | 400 kHz（最小时钟周期 2.5 µs） |
| 裸控制器 VDD | 1.65–3.3 V |
| 裸控制器 VBAT | 3.3–4.2 V |
| OLED 驱动 VCC | 7–15 V；模块通常由 charge pump 生成 |

上述三路电压是控制器/面板内部电源域，不能直接映射为成品模块的 `VCC` 排针规格。实际模块可能带稳压器、二极管或直接把排针接至 VDD，只有原理图或测量才能确认。

### I²C 帧格式

SSD1306 的 I²C 数据流在从地址后先发送 control byte：

- `Co=0` 表示后续连续字节属于同一传输流。
- `D/C#=0` 表示命令；常用 control byte 为 `0x00`。
- `D/C#=1` 表示 GDDRAM 数据；常用 control byte 为 `0x40`。

SDA/SCL 是开漏，需外部上拉。串行接口主要按写入使用，不应依赖从 OLED 读取控制器 ID 或状态来识别型号；很多四针板没有实现可靠的读回路径。

SSD1306 支持页寻址、水平寻址和垂直寻址。1024 字节全帧刷新最简单，但在 400 kHz 总线上还包含地址、控制字节和命令开销；动画应采用脏区域或分批刷新，避免阻塞同总线的传感器采样。

## SH1106 差异

Sino Wealth 编写的 [SH1106 Datasheet v2.3](https://resource.heltec.cn/download/oled/SH1106_V2.3.pdf) 由 Heltec 镜像托管。SH1106 内部显示 RAM 为 132×64，而常见模块玻璃只显示其中 128 列，因此许多板需要约 2 列的起始偏移；偏移仍应以实物显示结果为准。

| 项目 | SSD1306 | SH1106 常见行为 |
|---|---:|---:|
| 内部 RAM 宽度 | 128 列 | 132 列 |
| 常见可视宽度 | 128 列 | 128 列 |
| 列映射 | 通常从 0 开始 | 常见有列偏移 |
| I²C 地址 | `0x3C`/`0x3D` | 常见同为 `0x3C`/`0x3D` |
| I²C 上限 | 400 kHz | 400 kHz |
| 完整刷新方式 | 可用连续水平寻址 | 常按 page 设置列地址后写入 |

SH1106 与 SSD1306 的基础命令有重叠，但寻址和初始化细节不同。表现为左右缺列、画面平移、末端回卷或完全不亮时，不应只调 framebuffer；应先检查控制器型号、列偏移、charge-pump/DC-DC 配置、COM 扫描方向和复位时序。

## 实物识别清单

在把未知 OLED 接入 XIAO 前记录以下信息：

1. PCB 正反面、排线和控制器区域的清晰照片，以及卖家链接/BOM。
2. 排针实际顺序；常见 `GND/VCC/SCL/SDA` 并不是行业强制标准，接反可能损坏模块。
3. 断电测量 VCC 到芯片供电网络，确认有无 LDO/二极管；测量 SDA/SCL 上拉到哪个电源及阻值。
4. 以 3.3 V 上电，先扫描 `0x3C`/`0x3D`。扫描成功只证明总线 ACK，不证明控制器型号。
5. 分别用 SSD1306 与 SH1106 的保守初始化和测试图案检查四边、首末列、page 边界和像素偏移。不要快速反复切换可能改变面板高压配置的未知命令。

“0.96 英寸”也不能证明显示颜色、实际玻璃方向、像素尺寸、工作温度、模块上拉或复位电路。文档应绑定具体购买批次与照片，而不是把通用模块称为唯一硬件型号。

## ESP-IDF 集成建议

使用 [ESP-IDF 6.0.1 I²C Master Driver](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html) 的 `driver/i2c_master.h`，设备地址传未左移的 7 位值。建议驱动接口显式包含：控制器枚举、宽高、I²C 地址、列偏移、翻转方向、对比度和可选 reset GPIO。

工程实现时：

- 与 SHT40、SGP41、ENS160 共用一个 I²C bus handle，不要每个组件重复安装总线。
- OLED 刷新应在传感器 1 Hz 采样的空档分块执行，并对总线访问做串行化。
- 初始化后先清屏，再显示边缘框和角标测试图，以验证方向、偏移和 128×64 全范围。
- 若无独立 RESET 引脚，在上电后留出模块稳定时间；恢复 I²C 错误时不要假设软件命令等价于硬件复位。
- 把 framebuffer 内存位置和刷新任务栈纳入 ESP32-S3 内存预算；1024 字节只是像素数据，不含字体、绘图缓存和任务开销。

## 一手资料索引

- [Solomon Systech SSD1306 产品页](https://www.solomon-systech.com/zh-hans/product/SSD1306)
- [Solomon Systech OLED 产品目录](https://www.solomon-systech.com/product-category/oled-display)
- [SSD1306 Datasheet Rev 1.1（Waveshare 镜像）](https://files.waveshare.com/upload/a/af/SSD1306-Revision_1.1.pdf)
- [SH1106 Datasheet v2.3（Heltec 镜像）](https://resource.heltec.cn/download/oled/SH1106_V2.3.pdf)
- [ESP-IDF 6.0.1 I²C Master Driver](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html)
