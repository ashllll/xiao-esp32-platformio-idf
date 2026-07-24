# Waveshare ESP32-C6-LCD-1.47 一手资料核对

检索日期：2026-07-25（Asia/Shanghai）

> 本文是后续站点开发文档的研究底稿。它核对的是 **ESP32-C6-LCD-1.47 / ESP32-C6-LCD-1.47-M（无触摸）**，不能套用于名称相近的 `ESP32-C6-Touch-LCD-1.47`。除明确标为“社区”的链接外，事实优先来自 Waveshare 官方设计文件、官方 Demo、Espressif 官方文档及 LCD 模组资料。

## 1. 结论摘要

- 主控是 `ESP32-C6FH4`，板载 4 MB Flash；不是 8 MB 的 Touch 版本。高性能 RISC-V 核最高 160 MHz，低功耗 RISC-V 核最高 20 MHz。
- 屏幕是 1.47 英寸、172 × 320、262K 色 TFT，4 线 SPI。Waveshare 页面简称控制器为 ST7789；随板 LCD 模组手册明确为 `LBS147TC-IF15`、驱动 IC `ST7789V3`。
- LCD 与 microSD 共用 SPI 的 MOSI/SCLK（GPIO6/GPIO7），各自有独立 CS；同时访问必须正确仲裁总线并保持未使用设备 CS 为非激活电平。
- USB-C 直接连接 ESP32-C6 的原生 USB Serial/JTAG（GPIO12/13 的芯片 USB D-/D+ 内部功能），原理图没有 USB-UART 桥。Arduino `Serial` 输出需要正确启用 USB CDC/HWCDC；ESP-IDF 可使用 USB Serial/JTAG 控制台。
- 官方明确要求背光保持 **50% 或更低**，不要长期满亮；过热可能造成屏幕底部暗影。已有异常应冷却后刷入较低亮度程序。
- 官方旧 Wiki 含明显复用错误：Arduino 操作段落让用户选择 `ESP32S3 Dev Module`，而本板是 ESP32-C6。应使用与 ESP32-C6 对应的目标，不能照抄旧图文。

## 2. 型号、核心与无线能力

| 项目 | 已核验值 | 证据 |
|---|---|---|
| 产品 | ESP32-C6-LCD-1.47（SKU 28563）；ESP32-C6-LCD-1.47-M 带排针（SKU 30381） | [Waveshare 新文档主页](https://docs.waveshare.com/ESP32-C6-LCD-1.47) |
| SoC | ESP32-C6FH4，QFN32，封装内 4 MB Flash | [Waveshare 原理图](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf)、[Espressif ESP32-C6 数据手册 v1.5](https://documentation.espressif.com/esp32-c6_datasheet_en.pdf) |
| CPU/内存 | HP RISC-V 最高 160 MHz；LP RISC-V 最高 20 MHz；320 KB ROM、512 KB HP SRAM、16 KB LP SRAM | [Espressif 数据手册](https://documentation.espressif.com/esp32-c6_datasheet_en.pdf) |
| 无线 | 2.4 GHz Wi-Fi 6（802.11ax，并兼容 b/g/n）、Bluetooth LE、IEEE 802.15.4（Thread 1.3 / Zigbee 3.0） | [Espressif 数据手册](https://documentation.espressif.com/esp32-c6_datasheet_en.pdf) |
| 天线 | 板载陶瓷贴片天线与匹配网络 | [Waveshare 原理图](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf) |

准确性说明：Waveshare 页面写“Bluetooth 5”，Espressif 当前芯片数据手册写“Bluetooth LE 5.3 certified”。板级文档宜写“Bluetooth LE；芯片认证版本以 Espressif 当前数据手册为准”，避免把经典蓝牙或特定协议版本能力误写为板级验证结果。

## 3. 板载硬件与引脚

### 3.1 固定占用

| 功能 | ESP32-C6 GPIO | 说明 |
|---|---:|---|
| LCD MOSI / DIN | 6 | 与 microSD MOSI 共用 |
| LCD SCLK | 7 | 与 microSD SCLK 共用 |
| LCD CS | 14 | 低有效 |
| LCD D/C | 15 | 命令/数据选择 |
| LCD RESET | 21 | 低有效 |
| LCD 背光 PWM | 22 | 经 1 kΩ 驱动 SI2302 N-MOS；软件 PWM 控制 |
| WS2812B RGB 数据 | 8 | 板载单颗 RGB LED |
| microSD MISO | 5 | SPI 模式 |
| microSD MOSI | 6 | 与 LCD 共用 |
| microSD SCLK | 7 | 与 LCD 共用 |
| microSD CS | 4 | 低有效 |
| BOOT | 9 | 按键拉低；GPIO9 是芯片启动配置脚，复位采样期间不要外部强驱动 |
| RESET | CHIP_PU | 按键拉低复位，不是普通 GPIO |
| USB D- / D+ | 12 / 13（芯片功能） | 原生 USB Serial/JTAG；不应当作可无条件复用 GPIO |

来源：[Waveshare 新文档接口表](https://docs.waveshare.com/ESP32-C6-LCD-1.47)、[Waveshare 官方原理图](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf)、[Espressif ESP32-C6 数据手册](https://documentation.espressif.com/esp32-c6_datasheet_en.pdf)。

### 3.2 引出与冲突规则

- 原理图的排针/焊盘还引出 5V、3V3、GND，以及 GPIO0、1、2、3、18、19、20、23、UART TXD/RXD 等信号；具体物理位置必须以官方 Pinout 图或原理图为准。
- GPIO6/7 不是“空闲 SPI 引脚”：它们是 LCD 与 microSD 的共享总线。扩展第三个 SPI 设备时需要独立 CS、统一总线模式/频率管理，并验证各设备在 CS 非激活时是否释放 MISO。
- GPIO4、5、6、7、8、14、15、21、22 已被板载器件占用；GPIO9 兼作 BOOT；GPIO12/13 由 USB 使用。任何“多数 GPIO 已引出”的宣传描述都不代表这些引脚可以无冲突使用。
- microSD 仅按 SPI 1-bit 接线：D1/D2 未连接，不能把插槽当作 4-bit SDMMC 接口使用。

## 4. 电源、USB 与电气边界

- USB-C VBUS 为板上 `VCC_5V`，经 `ME6217C33M5G` LDO 生成 3.3 V；Waveshare 标称 LDO 最大 800 mA。这个数值是器件上限描述，不等于在所有输入电压、环境温度和 PCB 散热条件下都可持续输出 800 mA。
- Type-C 的 CC1/CC2 各有 5.1 kΩ 下拉，板作为 USB 设备取电。原理图未显示 USB-UART 桥或自动下载晶体管电路，USB 走 ESP32-C6 原生接口。
- 屏模组 `LBS147TC-IF15`：VDD 工作范围 2.5–3.3 V，逻辑接口 1.65–3.3 V；背光典型 3.0 V / 40 mA、最大表列 60 mA，2 颗白光 LED；屏模组工作温度 -20–70 ℃、存储 -30–80 ℃。
- 板上 3V3/5V 排针的可用外设电流没有官方板级降额曲线。不能只用“LDO 800 mA”推导外接负载预算，实际设计须计入 ESP32-C6 无线峰值、LCD/背光、microSD 瞬态、RGB LED、LDO 压差与热耗散。
- 不要从 3V3、5V 和 USB-C 多路反向供电；官方资料没有给出电源 OR-ing、反灌保护或外部供电切换规范。

来源：[Waveshare 官方原理图](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf)、[LBS147TC-IF15 LCD 模组手册](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/1.47_LCD_Manual.pdf)。

## 5. LCD 开发参数

| 参数 | 值 |
|---|---|
| 模组 | LBS147TC-IF15，Revision A |
| 面板 | Normally black TFT，全视角 |
| 分辨率 | 172(H) RGB × 320(V) |
| 色彩 | 262K 色 |
| 接口 | 4-line SPI |
| 驱动 IC | ST7789V3 |
| 有效区 | 17.3892 × 32.352 mm |
| LCD 模组尺寸 | 19.39 × 36.28 × 1.46 mm |
| 像素间距 | 0.0337(H) × 0.1011(V) mm |
| 典型亮度 | 350 cd/m² |

资料边界：

- 上表“模组尺寸”不是整块开发板外形尺寸。开发板机械尺寸仅在 Waveshare 尺寸图/3D Drawing 中以图形给出；本次未从可机器读取文本中可靠提取，因此不转录猜测值。
- LCD 模组手册封面/总表写 ST7789V3，但机械图一处出现 `STV7789V3/GC9307N` 文字。板级页面、官方原理图网名、Arduino/ESP-IDF Demo 均按 ST7789/ST7789T 驱动。量产或替换屏前仍应核对实物批次，不能据此假定所有批次控制器完全一致。
- 官方 Demo 的初始化序列、方向、显示偏移和颜色顺序是板级基准；只填“172×320 + ST7789”不保证显示窗口位置和色序正确。

来源：[LCD 模组手册](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/1.47_LCD_Manual.pdf)、[官方 Demo ZIP](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47-Demo.zip)。

## 6. 开发环境与官方示例

### 6.1 Arduino

- 官方最低要求：`esp32 by Espressif Systems >= 3.0.0`。
- 官方可复现组合：LVGL `8.3.10`、PNGdec `1.0.2`，均随 Demo 包提供离线副本。LVGL 8 驱动不能未经移植直接当作 LVGL 9 驱动使用。
- 示例：
  - `LVGL_Arduino`：初始化 Flash、LCD、LVGL、microSD，显示 Flash/SD/无线扫描等状态。
  - `LVGL_Image`（旧 Wiki 称 `LCD_Image`）：从 microSD 根目录读取并显示不超过 172 × 320 的 PNG。
- 官方 ZIP 中 Arduino 驱动确认 LCD 172 × 320、CS/DC/RST 为 14/15/21，SD CS 为 4；共享 SPI 使用 ESP32 Arduino 的 `SPI`/`SD` API。
- 目标应选支持 ESP32-C6 的通用 C6 开发板配置。旧 Wiki 中“ESP32S3 Dev Module”的文字和截图是复用错误，不可采用。

官方指南：[Working with Arduino](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Development-Environment-Setup-Arduino)。

### 6.2 ESP-IDF

- Waveshare 当前页面要求 ESP-IDF `>= 5.3.1`，示例截图使用 5.5.2，但同时要求使用与下载示例匹配的版本。
- 官方工程 `ESP32-C6-LCD-1.47-Test` 包含 ST7789T 面板驱动、LVGL、microSD、无线和 RGB 测试；目标必须是 `esp32c6`。
- Demo 功能：无线初始化、Flash 检测、RGB、SD、LCD、背光设为 50、LVGL 页面及 CPU 使用率展示。
- 常规命令行路径应是 `idf.py set-target esp32c6`、`idf.py build`、`idf.py -p <PORT> flash monitor`；VS Code 扩展只是同一 ESP-IDF 工作流的图形入口。
- 本项目是 PlatformIO + ESP-IDF 项目；导入官方 Demo 时应移植其 `main`/组件和 Kconfig/CMake 依赖，不要把 Arduino `.ino`、`SPI`、`SD` API 混入，除非明确引入 Arduino-as-component。

官方指南：[Working with ESP-IDF](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Development-Environment-Setup-ESP-IDF)、[Espressif ESP-IDF ESP32-C6 编程指南](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/)。

## 7. 烧录、串口与恢复

1. 使用可传数据的 USB-C 线连接；选择 ESP32-C6 目标和实际串口。
2. 正常情况下可通过原生 USB 下载。若后续无法连接或设备反复出现/消失：
   1. 按住 BOOT；
   2. 保持 BOOT 时按下并释放 RESET；
   3. 最后释放 BOOT；
   4. 重新选择端口并烧录。
3. ESP-IDF 烧录成功但停在 `waiting for download...` 时，按 RESET 或重新上电。
4. Arduino 无 `Serial` 输出时，核对 USB CDC On Boot/HWCDC 配置；直接 USB 板与 USB-UART 桥板的串口行为不同。
5. 官方 ZIP 提供 `Firmware/ESP32-C6-LCD-1.47-Test.bin` 工厂测试固件，但单一 BIN 的烧录地址必须遵循 Waveshare 的固件烧录说明或 ZIP 附带配置，不能盲猜为任意地址。
6. “编译成功/烧录成功”不等于硬件通过；至少应核对 LCD 全屏色块/方向/边缘、背光 PWM、microSD 读写、RGB、无线扫描、复位与重复烧录。

来源：[Waveshare FAQ](https://docs.waveshare.com/ESP32-C6-LCD-1.47/FAQ)、[Waveshare ESP-IDF 指南](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Development-Environment-Setup-ESP-IDF)。

## 8. 必须保留的注意事项

- 长期使用背光不超过 50%；不得将满亮当成默认生产设置。
- 焊接无排针版本时不要拆屏；倾斜烙铁直接焊排针。屏与背壳为胶粘一体结构，不应撬开。
- BOOT/GPIO9 是启动配置脚，外接电路不能在复位采样阶段把它拉到错误状态。
- 原生 USB 占用对应芯片 USB 信号；改作 GPIO 会失去下载/日志通道并可能造成总线电气冲突。
- LCD 和 microSD 共享 SPI，切换设备前确保各 CS 状态、事务频率和 SPI 模式正确。
- microSD 可选：FAQ 明确板可在没有 TF 卡时使用，但依赖 SD 的 Demo 页面会显示初始化失败；应用应优雅处理卡缺失。
- 该板无触摸、无 IMU、无电池充电电路。名称相似的 Touch 版本资料不得混入。

## 9. 来源冲突与未确认项

| 项目 | 状态与处理 |
|---|---|
| 用户给出的中文 Wiki | `waveshare.net/wiki` 当前抓取结果只剩空壳/旧版本入口；以 Waveshare 新文档平台和国际官方 Wiki 的可读取内容补全。 |
| Arduino 板型 | 旧 Wiki 写 ESP32S3 Dev Module，和芯片、原理图、ESP-IDF 目标冲突；判定为模板复用错误，使用 ESP32-C6 目标。 |
| LCD 控制器 | 页面写 ST7789，模组总表写 ST7789V3，机械图又出现 STV7789V3/GC9307N；当前官方代码走 ST7789T 系列初始化。实物批次/替代料未确认。 |
| Bluetooth 版本措辞 | Waveshare 简写 Bluetooth 5；Espressif 当前数据手册为 Bluetooth LE 5.3 certified。不可写成支持经典蓝牙。 |
| 整板机械尺寸 | 官方尺寸图和 3D RAR 可下载，但页面没有文本值；未可靠提取，后续发布页面应嵌图或人工复核 CAD，不猜值。 |
| 最大 SPI 时钟 | 官方 Demo 含具体运行配置，但 LCD 模组手册的时序表没有在本次文本抽取中形成可靠完整数值；不把示例频率升级为器件保证值。 |
| 5V/3V3 外供能力 | 无板级热降额、反灌和最大外设电流规范；保持“未给出”，不得从 LDO 800 mA 直接推算。 |
| Arduino/IDF 精确版本 | 页面只给最低版本，ZIP 是一个时间点快照且未发现发布版本清单；复现时应记录 ZIP 哈希、Arduino core/IDF 实际版本并本地构建。 |
| 原厂身份 | LCD PDF 标注 Limito Technology，但 Waveshare 未提供可独立验证的 ST7789V3 原厂控制器数据手册；屏模组 PDF属于 Waveshare 随板资料，不足以解决所有驱动 IC 批次问题。 |

## 10. 官方资源索引

所有链接检索于 2026-07-25。

- [用户指定的 Waveshare 中文 Wiki](http://www.waveshare.net/wiki/ESP32-C6-LCD-1.47)
- [Waveshare 新文档主页](https://docs.waveshare.com/ESP32-C6-LCD-1.47)
- [Waveshare Resources](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Resources-And-Documents)
- [Waveshare Arduino 指南](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Development-Environment-Setup-Arduino)
- [Waveshare ESP-IDF 指南](https://docs.waveshare.com/ESP32-C6-LCD-1.47/Development-Environment-Setup-ESP-IDF)
- [Waveshare FAQ](https://docs.waveshare.com/ESP32-C6-LCD-1.47/FAQ)
- [Waveshare 官方原理图 PDF](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf)
- [Waveshare 官方 3D Drawing RAR](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47-3D_Drawing.rar)
- [Waveshare 官方 Demo ZIP](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47-Demo.zip)
- [LBS147TC-IF15 LCD 模组手册](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/1.47_LCD_Manual.pdf)
- [Espressif ESP32-C6 数据手册 v1.5](https://documentation.espressif.com/esp32-c6_datasheet_en.pdf)
- [Espressif ESP32-C6 技术参考手册（当前官方入口）](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/api-reference/index.html)
- [Espressif ESP-IDF ESP32-C6 编程指南](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/)
- [Espressif Arduino-ESP32 文档](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [Waveshare ESP32-display-support GitHub](https://github.com/waveshareteam/ESP32-display-support)
- [LVGL 8.3 文档](https://docs.lvgl.io/8.3/)

社区案例只用于扩展思路，不作为硬件事实依据：

- [VolosR/WaveShareC6lvglexample](https://github.com/VolosR/WaveShareC6lvglexample)
- [thelastoutpostworkshop/ESP32-C6-LCD-1.47_LVGL9_Crypto_Monitor](https://github.com/thelastoutpostworkshop/ESP32-C6-LCD-1.47_LVGL9_Crypto_Monitor)
- [thelastoutpostworkshop/ESP32-C6-LCD-1.47_video_player](https://github.com/thelastoutpostworkshop/ESP32-C6-LCD-1.47_video_player)

## 11. 后续发布前验收清单

- [ ] 从官方 ZIP 建立 SHA-256 清单并记录 ZIP 内示例版本。
- [ ] 在本仓库实际 PlatformIO ESP-IDF 环境完成 C6 构建，而不是只验证上游 VS Code 工程。
- [ ] 实物确认丝印型号、SoC 标记、屏批次及整板尺寸。
- [ ] 实物测试 50% 背光下温升与暗影，确认 PWM 极性及占空比含义。
- [ ] LCD 做红绿蓝白黑、四角、文字方向和全屏边界测试。
- [ ] microSD 做无卡、插卡、读写、与 LCD 并发切换测试。
- [ ] 重复验证普通复位、BOOT 下载模式、USB 串口日志和固件恢复。
- [ ] 无线能力分别按 Wi-Fi、BLE、802.15.4 的实际固件与法规目标验收，不能只凭芯片支持列表宣称产品功能。
