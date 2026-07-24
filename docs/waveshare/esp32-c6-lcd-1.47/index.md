# Waveshare ESP32-C6-LCD-1.47

资料核对日期：2026-07-25

本目录独立整理 Waveshare `ESP32-C6-LCD-1.47`（SKU 28563）与带排针的 `ESP32-C6-LCD-1.47-M`（SKU 30381）。它们是集成屏幕的 ESP32-C6 开发板，不是 Seeed XIAO ESP32C6，也不是带触摸、8 MB Flash 和 IMU 的 `ESP32-C6-Touch-LCD-1.47`。

!!! warning "先保护屏幕"
    Waveshare 要求背光保持在 **50% 或以下**，不要长时间满亮度运行。过热可能造成屏幕底部暗影；出现异常时应断电冷却，再烧录降低亮度的程序。焊接排针时不要拆屏，应倾斜烙铁直接焊接。

## 已核实规格

| 项目 | 结论 | 证据 |
|---|---|---|
| MCU | ESP32-C6FH4，最高 160 MHz 的 HP RISC-V 核及最高 20 MHz 的 LP RISC-V 核 | Waveshare 产品页、板级原理图、Espressif 数据手册 |
| 存储 | 4 MB 封装内 Flash；512 KB HP SRAM、16 KB LP SRAM、320 KB ROM | Waveshare 产品页、Espressif 数据手册 |
| 无线 | 2.4 GHz Wi-Fi 6、Bluetooth 5 LE、IEEE 802.15.4 | Espressif 数据手册；Waveshare 页面未完整列出 802.15.4 |
| 显示 | 1.47 英寸 TFT、172×320、262K 色、ST7789 系列、4 线 SPI | Waveshare 页面、LCD 手册、官方示例 |
| 存储卡 | 板载 microSD/TF 卡槽，SPI 模式 | 原理图、官方示例 |
| USB | USB Type-C，ESP32-C6 原生 USB Serial/JTAG | 原理图、Espressif 文档 |
| 其他 | GPIO8 可编程 RGB LED；BOOT 与 RESET 按键；板载陶瓷天线 | 产品页、原理图、示例 |

“262K 色”描述面板能力；官方示例以 RGB565（16 bit/pixel）传输。`172×320×2 = 110,080` 字节，因此完整双缓冲会明显占用片内 SRAM。官方 ESP-IDF 示例使用 20 行局部缓冲，而不是两个完整 framebuffer。

## 阅读路线

1. [硬件与引脚](hardware.md)：电源、SPI 共享、LCD、TF 卡、RGB LED、USB 与启动脚。
2. [开发环境与示例](development.md)：Arduino、ESP-IDF、PlatformIO 边界及厂商示例内容。
3. [故障排查与验收](troubleshooting.md)：下载模式、黑屏、暗影、TF 卡和真机验收。
4. 一手资料核对页：记录下载包、原理图和厂商网页之间的差异。

## 重要边界

- 本仓库现有 `xiao_esp32c6` 环境的板定义和引脚表只适用于 XIAO，不可直接烧录到本板。
- Waveshare 官方包同时含 Arduino 与原生 ESP-IDF 工程；不能把 Arduino API 复制进本仓库的纯 ESP-IDF 应用。
- 文档已静态核对，但本次没有连接实板；编译、文档构建和来源交叉核对不等于屏幕、TF 卡、RF 或热行为通过真机验收。

## 官方入口

- [Waveshare 当前文档](https://docs.waveshare.com/ESP32-C6-LCD-1.47)
- [硬件原理图](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47_schemetics.pdf)
- [官方示例包](https://files.waveshare.com/wiki/ESP32-C6-LCD-1.47/ESP32-C6-LCD-1.47-Demo.zip)
- [ESP32-C6 数据手册](https://documentation.espressif.com/esp32-c6_datasheet_en.html)
- [ESP-IDF ESP32-C6 编程指南](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/)
