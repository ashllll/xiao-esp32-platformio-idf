# 硬件与引脚

## 板载连接

| 功能 | 信号 | ESP32-C6 GPIO | 备注 |
|---|---|---:|---|
| LCD | MOSI | 6 | 与 TF 卡共享 |
| LCD | SCLK | 7 | 与 TF 卡共享 |
| LCD | CS | 14 | LCD 独立片选 |
| LCD | D/C | 15 | 命令/数据选择 |
| LCD | RESET | 21 | 低电平复位 |
| LCD | Backlight | 22 | 高有效 PWM；限制到 50% |
| TF 卡 | MISO | 5 | LCD 不使用 MISO |
| TF 卡 | MOSI | 6 | 共享 SPI |
| TF 卡 | SCLK | 7 | 共享 SPI |
| TF 卡 | CS | 4 | TF 独立片选 |
| RGB LED | DIN | 8 | 单线可编程 RGB |
| USB | D− / D+ | 12 / 13 | 原生 USB，勿作为普通扩展 GPIO |
| BOOT | 按键 | 9 | 按下拉低；也是启动配置脚 |
| RESET | 按键 | EN | 按下复位 |

该表由 Waveshare 页面、两页原理图和官方 Arduino/ESP-IDF 源码交叉核对。LCD 和 TF 共用 `GPIO6/7`，必须由同一个 SPI bus 管理并使用独立 CS；不能让两个驱动分别重新初始化总线。TF 卡仅接 SPI 所需四线，`SD_D1/SD_D2` 未连接，不能配置为 SDMMC 4-bit。

## LCD

- 逻辑分辨率为竖屏 172×320；官方驱动采用 16-bit RGB565、BGR 顺序和显示反色。
- 官方 ESP-IDF 示例把 SPI pixel clock 设为 12 MHz。Arduino 源码使用默认 SPI transaction 参数；不要把网页或第三方项目中的更高频率当成板级保证。
- 面板有列/行偏移和专用初始化序列。移植时应保留官方 ST7789 初始化表，并用四角测试图检查方向、裁剪、颜色顺序和偏移。
- 背光由 GPIO22 经 MOSFET 驱动，不应从 GPIO 直接给 LED 供电。PWM 高有效。

## 电源

USB-C VBUS 进入 `VCC_5V`，再由 `ME6217C33M5G` LDO 生成 3.3 V。Waveshare 把 LDO 标为最大 800 mA；该额定值不是整板在所有温度下可持续提供 800 mA 的证明。屏幕背光、无线峰值、TF 卡启动电流、LDO 压差和 PCB 散热都需要实测。

扩展排针同时引出 5 V、3.3 V 与 GND。ESP32-C6 GPIO 只按 3.3 V 逻辑使用，不能接 5 V 信号。不要同时从 USB 和外部 5 V 回灌，除非已根据原理图和电源时序确认供电方案。

## 可用引脚与约束

原理图还引出 GPIO0、1、2、3、10、11、18、19、20、23。分配前必须查 [ESP32-C6 数据手册的 IO MUX、启动配置和限制](https://documentation.espressif.com/esp32-c6_datasheet_en.html)：

- GPIO8 已占用 RGB；GPIO9 已占用 BOOT。
- GPIO12/13 已占用 USB。
- GPIO4–7、14、15、21、22 已被显示或 TF 卡使用。
- 无线功能不需要用户分配射频 GPIO，但布局、外壳和靠近陶瓷天线的金属会影响性能。

## 型号识别

下单或接线前确认丝印和 SKU。无触摸版使用 ESP32-C6FH4、4 MB Flash、ST7789 LCD；触摸版页面列出的 ESP32-C6FH8、8 MB Flash、JD9853、AXS5106L 和 QMI8658 不适用于本板。
