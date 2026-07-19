# GY-SGP41 VOC/NOx 气体传感器

## 先纠正商品标题

常见商品标题会把 GY-SGP41 写成“TVOC、eCO₂、空气质量、二氧化碳测量”。这与 Sensirion SGP41 的原厂接口不符：SGP41 提供 `SRAW_VOC` 和 `SRAW_NOX` 原始信号，配合官方 Gas Index Algorithm 得到 VOC Index 与 NOx Index；它**不直接输出 TVOC 浓度、eCO₂ 或 CO₂**。需要真实 CO₂ 浓度时，应选择基于 NDIR 或光声原理并明确给出 CO₂ 指标的传感器。

本文适用于带 SGP41 芯片的通用 GY 模块。由于“GY-SGP41”不是唯一厂商料号，板载稳压、上拉、电平转换和引脚顺序必须以收到的实物和原理图为准。

## 电气与总线

| 项目 | 已确认的芯片参数 | 模块使用规则 |
|---|---|---|
| 芯片供电 | 1.7–3.6 V | XIAO 项目统一从 `3V3` 供电；未核对原理图前不要接 5 V |
| 接口 | I²C，Standard/Fast mode | 先用 100 kHz；共享总线可在核对上拉后使用 400 kHz |
| 7 位地址 | `0x59` | 地址固定，无法通过引脚改址 |
| 输出 | 原始 VOC、NOx 信号 | 不是 ppm/ppb，也不是 CO₂ 浓度 |
| 采样节拍 | 每 1 秒执行一次测量 | 官方 Gas Index Algorithm 依赖稳定的 1 s 节拍 |

通用模块可能已经装有上拉。并联 ENS160、SHT40 和 OLED 前，测量总线是否上拉到 3.3 V，并核算所有板载上拉并联后的等效阻值。

## 接线

| GY-SGP41 | XIAO | GPIO（C3 / S3 / C6） | 说明 |
|---|---|---:|---|
| `VCC`/`VIN` | `3V3` | — | 保守使用 3.3 V |
| `GND` | `GND` | — | 必须共地 |
| `SDA` | D4 | 6 / 5 / 22 | `XIAO_I2C_SDA` |
| `SCL` | D5 | 7 / 6 / 23 | `XIAO_I2C_SCL` |

## 测量流程

1. 初始化共享 I²C 总线并探测 `0x59`。
2. 每次复位或加热器关闭后，连续执行 conditioning 命令 10 秒；原厂说明该阶段不应超过 10 秒。
3. 每秒写入当前相对湿度和温度补偿值，再执行 raw-signals 测量。
4. 等待测量完成，读取两个 16 位结果；每个 2 字节数据字后都有 1 字节 CRC。
5. 用 CRC-8（多项式 `0x31`、初值 `0xFF`）校验后，将原始值输入官方 Gas Index Algorithm。
6. 保存有效性/运行时间状态；不要把上电后的首批 index 当成已稳定的绝对浓度。

Sensirion 的通用 Embedded I²C 驱动是可移植 C 代码，但仍需为 ESP-IDF 实现其 HAL、时钟和总线访问。Gas Index Algorithm 是另一个独立仓库；固定具体提交或版本，并为 1 s 调度和状态持久化增加测试，不能只复制 Arduino 示例。

## 温湿度补偿

SGP41 命令接受外部相对湿度和温度。推荐使用同一气流位置的 SHT40 实测值。没有外部数据时，原厂默认字在数值上约对应 50 %RH 和 25 °C，但协议明确用它们表示“关闭补偿”；不能把日志写成传感器正在用固定环境值补偿，更不能把默认字当成环境测量值。

补偿值写入和返回数据都带 Sensirion CRC。产品代码必须拒绝 CRC 错误，不能把损坏的数据继续送入算法。

## 最小验证

- 地址扫描只发现一个 `0x59`，无间歇 NACK。
- conditioning 恰好运行推荐的 10 s，随后保持 1 Hz 测量，不反复开关加热器。
- 故意篡改一个响应字节时，驱动能报告 CRC 错误。
- 日志名称使用 `raw_voc`、`raw_nox`、`voc_index`、`nox_index`，不使用 `co2_ppm` 或 `tvoc_ppb`。
- 对照洁净空气和一次短暂气味事件观察趋势；这只验证响应链路，不构成气体浓度校准。

原厂对开机响应和算法稳定给出了不同时间尺度；具体产品判定应按数据表的 switch-on behavior 表执行。不要用“数十秒有响应”替代完整稳定时间，也不要把趋势验证写成准确度验证。

## 来源与核对

- [Sensirion SGP41 产品页](https://sensirion.com/products/catalog/SGP41)
- [Sensirion SGP41 数据表（v1.0，2021-12）](https://sensirion.com/media/documents/5FE8673C/61E96F50/Sensirion_Gas_Sensors_Datasheet_SGP41.pdf)
- [Sensirion Embedded I²C SGP41 驱动](https://github.com/Sensirion/embedded-i2c-sgp41)
- [Sensirion Gas Index Algorithm](https://github.com/Sensirion/gas-index-algorithm)
- [ESP-IDF 6.0.1 I²C 主机文档](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html)
- [SGP41 一手资料核对](../research/sgp41-primary-sources.md)

检索日期：2026-07-19。本文没有获得该“GY-SGP41”商品的唯一原理图，因此没有宣称模块支持 5 V，也没有给出未核实的板级电流。
