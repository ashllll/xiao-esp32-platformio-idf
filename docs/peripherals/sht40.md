# SHT40 Qwiic 温湿度传感器

## 适用范围

SHT40 是数字温湿度传感器，可独立记录环境，也适合为 ENS160、SGP41 提供温湿度补偿。“SHT40 For QWIIC”是通用商品描述，不足以确定传感器后缀、I²C 地址、板载稳压或电平转换电路；本文采用 Sensirion 裸芯片数据表与 Qwiic 的 3.3 V 规则，并把未知板级参数明确留待实物核对。

SHT40 的典型精度为 ±1.8 %RH、±0.2 °C；准确度随温湿度区间变化，应以数据表误差图和具体后缀为准，不能把典型值当作全范围最大误差。

## 电气与地址

| 项目 | 数值/规则 |
|---|---|
| 裸芯片供电 | 1.08–3.6 V；本项目按 3.3 V 使用 |
| 总线 | I²C Standard/Fast/Fast-mode Plus；本项目先用 100 kHz |
| 常见地址 | SHT40-AD1B `0x44`；BD1B `0x45`；CD1B `0x46` |
| 响应校验 | 每个 16 位数据字后跟 CRC-8；多项式 `0x31`、初值 `0xFF` |
| Qwiic 电源/逻辑 | 3.3 V；不要因模块接口外形相同就接入 5 V I²C |

多数通用小板使用 `0x44`，但驱动应先扫描并记录实际地址，不能悄悄尝试多个地址后隐藏硬件型号。多个 Qwiic 模块会把上拉电阻并联；总线不稳定时先核对等效上拉与线长。

## 接线

| 模块/Qwiic | XIAO | GPIO（C3 / S3 / C6） | Qwiic 线色 |
|---|---|---:|---|
| `VCC`/`3V3` | `3V3` | — | 红 |
| `GND` | `GND` | — | 黑 |
| `SDA` | D4 | 6 / 5 / 22 | 蓝 |
| `SCL` | D5 | 7 / 6 / 23 | 黄 |

线色只用于帮助检查标准 Qwiic 线缆，最终仍以连接器丝印和万用表通断测量为准。

## ESP-IDF 读取流程

高精度单次测量的基本流程：

1. 向设备发送命令 `0xFD`。
2. 等待数据表规定的最大测量时间；高精度模式最大 8.3 ms。
3. 读取 6 字节：温度 MSB、LSB、CRC，湿度 MSB、LSB、CRC。
4. 分别验证两个 CRC；任一失败则丢弃整次样本。
5. 将无符号 16 位原始值换算为：

```text
temperature_c = -45 + 175 * raw_temperature / 65535
relative_humidity = -6 + 125 * raw_humidity / 65535
```

正常展示时可将相对湿度限制到 0–100 %RH，但应保留原始值和 CRC/错误状态供诊断。中、低精度单次测量命令分别为 `0xF6`、`0xE0`；读取序列号为 `0x89`，软复位为 `0x94`。SHT4x 不使用 clock stretching。

官方 Arduino 库不能直接作为本工程的纯 ESP-IDF 组件。实现原生组件时使用 `driver/i2c_master.h`，在 CMake 中依赖 `esp_driver_i2c`，并让共享总线层负责设备生命周期。

## 加热器限制

SHT40 的片上加热器用于应对冷凝/高湿爬移，不是提高日常测量精度的常开功能：

- 加热期间及其后续热恢复阶段的读数不能代表环境温湿度。
- 最高档在 3.3 V 下的供电电流为典型 60 mA、最大 100 mA；供电与调度必须按最大值留出裕量。
- 原厂规定加热器总占空比必须小于 10%，并且芯片温度不能超过 125 °C。

应用应把加热动作记录为数据质量事件，避免把受自热影响的值送给 ENS160/SGP41 做补偿。

## 最小验证

- 扫描并记录 `0x44`、`0x45` 或 `0x46` 中的实际地址。
- 连续读取序列号，确认连接和 CRC 稳定。
- 在室温下与可信参考温湿度计并排静置，比较一段时间的趋势；手握模块会带来自热和湿气干扰。
- 注入 CRC 错误并确认样本被拒绝。
- 若启用加热器，验证占空比上限和无效数据标记。

## 来源与核对

- [Sensirion SHT40 产品页](https://sensirion.com/products/catalog/SHT40)
- [Sensirion SHT4x 数据表（v7.1，2025-03）](https://sensirion.com/resource/datasheet/sht4x)
- [Sensirion Embedded I²C SHT4x 驱动](https://github.com/Sensirion/embedded-i2c-sht4x)（需要适配 ESP-IDF HAL）
- [SparkFun Qwiic 连接系统](https://www.sparkfun.com/qwiic)
- [ESP-IDF 6.0.1 I²C 主机文档](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html)
- [SHT40 一手资料核对](../research/sht40-primary-sources.md)

检索日期：2026-07-19。通用模块的原理图和传感器顶标仍需以收到的实物补齐；在此之前不宣称支持 5 V。
