# 优信电子 OPT101 模块一手资料核对

检索日期：2026-07-19

“优信电子 OPT101 模块”目前没有可唯一确认的原理图、产品型号或稳定官网资料，因此不能把未知 PCB 的输入电压、反馈增益、输出保护和引脚定义当作已知事实。本文以 Texas Instruments OPT101 裸芯片数据表为边界，并给出连接 XIAO ESP32-S3 ADC 前必须完成的板级核对。

## 结论摘要

- OPT101 是集成光电二极管与跨阻放大器的模拟光强传感器，输出随入射辐射功率增加；它不是数字 I²C 器件，也不是经过人眼光视效率校准的 lux 传感器。
- 裸芯片支持 2.7–36 V 单电源，但这不等于未知模块可把高压输出直接接 ESP32 ADC。OPT101 输出高端可接近 `VS−1.3 V`，模块用 5 V 或更高供电时必须确认分压、钳位与 ADC 绝对最大值。
- 内部 1 MΩ 反馈时，650 nm 典型光谱响应度约 0.45 A/W，输出响应度约 0.45 V/µW。该值随波长变化，不能用一个固定系数把任意光源换算为 lux。
- ADC 原始码不等于电压。ESP-IDF 应使用 ADC oneshot 与 calibration API，采集暗电平、校准毫伏值，并通过参考仪表和目标光源做系统级拟合。
- 未查清优信模块电路前，不能确认排针、供电范围、输出增益或所谓“照度公式”。应先追踪 PCB 走线和测量静态输出。

## 裸芯片规格

参数来自 [Texas Instruments OPT101 产品页](https://www.ti.com/product/OPT101) 与 [OPT101 Datasheet Rev B](https://www.ti.com/lit/ds/symlink/opt101.pdf)（SBBS002B，1994-01，2015-06 修订）。

| 项目 | OPT101 典型/规定值 | 解释 |
|---|---:|---|
| 电源范围 | 2.7–36 V | 裸芯片；模块和 MCU 接口另行确认 |
| 暗态静态电流 | 120 µA 典型 | 不含外部负载 |
| 规定工作温度 | 0–70 °C | 数据表电气特性条件 |
| 光谱响应范围 | 约 300–1100 nm | 峰值偏近红外，不匹配人眼 V(λ) |
| 650 nm 响应度 | 0.45 A/W 典型 | 光电流/辐射功率 |
| 650 nm 输出响应度 | 0.45 V/µW 典型 | 使用内部 1 MΩ 反馈 |
| 响应度器件差异 | 约 ±5% | 仍不包含光源光谱与机械误差 |
| 内部反馈电阻 | 1 MΩ | 可配合外部反馈网络改变增益/带宽 |
| 内部反馈时带宽 | 14 kHz 典型 | 更高增益通常降低带宽 |
| 暗输出 | 7.5 mV 典型，5–10 mV | 应实测并做暗基线 |
| 输出高摆幅 | 至少 `VS−1.3 V`，典型 `VS−1.15 V` | 接 ADC 时按最坏情况保护 |
| 光敏面积 | 2.29 × 2.29 mm | 封装窗口、遮光罩会改变系统响应 |

数据表的 0.45 V/µW 是特定波长和反馈配置下的辐射功率响应，不是 lux 响应。白光 LED、太阳光、荧光灯和红外光含有不同光谱；同样 lux 可产生不同 OPT101 输出，同样输出也可能对应不同 lux。

## 裸芯片引脚与典型连接

| 引脚 | 名称 | 用途 |
|---:|---|---|
| 1 | `VS` | 正电源 |
| 2 | `-IN` | 运放反相输入；外部反馈配置点 |
| 3 | `-V` | 负电源；单电源使用时接地 |
| 4 | `1MΩ` | 内部反馈电阻端，典型接输出形成 1 MΩ TIA |
| 5 | `OUT` | 模拟电压输出 |
| 6、7 | NC | 不连接 |
| 8 | `COMMON` | 光电二极管阳极/公共端；典型接地 |

VS 与 -V 之间应在芯片附近放置 0.01–0.1 µF 去耦。裸芯片典型连接不能替代模块原理图：优信 PCB 可能已经连接 1 MΩ、增加电容或电阻、改变共模点，甚至对 VOUT 做分压。用万用表断电追踪后再接线。

## 接入 XIAO ESP32-S3 前的电气检查

1. 记录模块正反面和所有丝印，确认实际 OPT101 封装方向及排针对应的 `VCC/GND/OUT`。
2. 断电测量反馈网络、VOUT 到排针之间的串联/分压/钳位器件，以及电源去耦。
3. 用限流电源先以 3.3 V 供电；遮光时测暗输出，逐步增加稳定光照并确认输出单调、无振荡。
4. 在最强预期光照下测 VOUT 上限。必须保证它在 XIAO ADC 允许输入范围内，并低于 GPIO 绝对最大额定值。
5. 若模块必须以 5 V 或更高电压供电，设计经过计算的分压和限流/钳位，不要依赖 ESP32 内部保护二极管吸收正常工作电流。

即使裸 OPT101 可以 36 V 供电，3.3 V 供电通常更容易和 ESP32 安全连接；但它会降低最大不饱和光强。最终电源、增益与分压应从目标光照动态范围倒推，并以最坏公差验证。

## ESP-IDF ADC 采集建议

ESP-IDF 6.0.1 使用 [`esp_adc/adc_oneshot.h`](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/adc/adc_oneshot.html) 进行单次采样，并使用 [ADC Calibration Driver](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/adc/adc_calibration.html) 将 raw code 转成经芯片校准的毫伏值。

建议实现：

- 优先选择 ADC1 可用引脚，避免 ADC2 与无线功能的资源冲突；具体 GPIO/通道必须以所用 XIAO ESP32-S3 变体和本工程 `xiao_pins.h` 为准。
- 按实测 VOUT 范围选择衰减与位宽；不要把配置的标称衰减范围误当成 GPIO 绝对最大值。
- 定时采集多点，使用中值或均值抑制噪声，并保留未滤波 raw、校准 mV、供电电压和增益配置。
- 在完全遮光、若干已知稳定光级以及接近饱和处建立校准点。暗基线应按单块模块保存，必要时补偿温漂。
- 模拟线保持短、远离 OLED charge pump、Wi-Fi 天线和高速 I²C；在确认稳定性后才决定是否增加 RC 滤波，避免无意降低所需带宽。

如果应用只需要“相对亮度”，可输出扣除暗电平后的校准电压或归一化百分比，并明确饱和状态。如果需要 lux，必须对目标光源、扩散片/外壳、安装角度和视场固定后，与可追溯照度计进行多点拟合；更换光谱或光学结构后重新校准。不能仅用 `0.45 V/µW` 推导通用 lux。

## 建议的数据模型

一次测量至少包含：

```text
adc_raw
adc_millivolts
dark_offset_millivolts
signal_millivolts
supply_millivolts
gain_configuration
saturated
calibration_id
relative_light 或 calibrated_lux（仅在完成对应校准时）
```

这能避免把未经校准的 ADC 码误标成 lux，也便于模块换批次、反馈网络变化后追踪数据口径。

## 一手资料索引

- [Texas Instruments OPT101 产品页](https://www.ti.com/product/OPT101)
- [Texas Instruments OPT101 Datasheet Rev B](https://www.ti.com/lit/ds/symlink/opt101.pdf)
- [ESP-IDF 6.0.1 ADC Oneshot Mode Driver](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/adc/adc_oneshot.html)
- [ESP-IDF 6.0.1 ADC Calibration Driver](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/adc/adc_calibration.html)
