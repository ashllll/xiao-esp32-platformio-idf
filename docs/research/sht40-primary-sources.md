# QWIIC SHT40 一手资料核对

检索日期：2026-07-19

`QWIIC SHT40` 目前只能确定为带四线 I²C 接口的 SHT40 模块描述，不能据此唯一确定制造商、PCB 版本或电路。本文仅把 Sensirion SHT40 芯片能力作为已证实事实；模块的输入电压范围、稳压器、电平转换、上拉阻值、连接器针序和地址必须按实物丝印、卖家原理图或测量结果复核。

## 结论摘要

- SHT40 直接测量温度与相对湿度，不输出 TVOC、NOx、AQI 或 eCO2。
- 裸 SHT40 供电范围为 1.08–3.6 V，典型 3.3 V；GPIO 电平随 VDD，任何引脚都不能高于 `VDD+0.3 V`。在未知模块上不能因为有 Qwiic 类连接器就推断支持 5 V。
- SHT40 地址由具体订货型号在制造时决定：A/B/C 变体分别为 `0x44`、`0x45`、`0x46`。多数示例用 `0x44`，但应探测实物并读取序列号，不能把 `0x44` 当作所有模块的硬性事实。
- 每个 16 位返回字后都有 CRC-8；生产代码必须校验温度和湿度两个 CRC，不能只读取 4 个数据字节。
- 内置 heater 用于处理凝露/高湿，不是环境加热器，也不是常规测量时必须开启。heater 开启时规格不成立，最高档电流可达 100 mA，最大占空比小于 10%。
- Sensirion 官方嵌入式 C 驱动可用于 ESP-IDF，但其 I²C HAL 是平台模板，需要适配到本工程共享的 `i2c_master` 总线；不能直接复制 Linux 示例。

## 裸芯片规格

以下数据来自 [Sensirion SHT4x Datasheet v7.1](https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf)（2025-03）。它们不自动等于未知 QWIIC 模块的外部规格。

| 项目 | SHT40 芯片值 | 备注 |
|---|---:|---|
| VDD | 1.08–3.6 V，典型 3.3 V | §3 Table 4 |
| 相对湿度范围 | 0–100 %RH | 完整量程；推荐长期环境见下文 |
| SHT40 典型 RH 精度 | ±1.8 %RH | 具体最大误差随 RH/温度变化，应看数据表曲线 |
| RH 响应时间 | 4 s，τ63% | 25 °C、1 m/s 气流；模块结构会改变实际响应 |
| 温度范围 | -40–125 °C | 规格范围不代表模块材料也适合该范围 |
| SHT40 典型温度精度 | ±0.2 °C | 最大误差随温度变化 |
| 温度响应时间 | 2 s，τ63% | 受 PCB 导热、自热和外壳影响 |
| Idle 电流 | 0.08 µA 典型（25 °C） | heater 关闭 |
| 1 Hz 平均电流 | 高/中/低重复性约 2.2/1.2/0.4 µA | heater 关闭 |
| 测量瞬时电流 | 320 µA 典型、500 µA 最大 | heater 关闭 |
| 上电就绪时间 | 最大 1 ms | 达到最低供电电压后 |

最佳性能的推荐环境是 5–60 °C、20–80 %RH。长期处于高湿或其他极端环境可能带来暂时偏移并加速老化；返回正常范围后通常会自行恢复。0–100 %RH 和 -40–125 °C 是测量/工作范围，不应写成“全范围始终保持典型精度”。[SHT4x Datasheet §2.3](https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf)

## 地址、命令与 CRC

SHT40 的 I²C 地址取决于完整订货型号，而不是软件寄存器：

| 型号前缀 | 7 位地址 |
|---|---:|
| SHT40-A… | `0x44` |
| SHT40-B… | `0x45` |
| SHT40-C… | `0x46` |

地址依据：[SHT4x Datasheet §9、§10](https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf)。官方当前 [embedded-i2c-sht4x](https://github.com/Sensirion/embedded-i2c-sht4x) README 仅概括列出 SHT40 的 `0x44`/`0x45`，因此地址全集以更新且型号化的数据表 v7.1 为准。

常用的一字节命令：

| 命令 | 含义 | 典型等待/响应 |
|---:|---|---|
| `0xFD` | 高重复性 T/RH 测量 | 最大 8.3 ms；返回 6 字节 |
| `0xF6` | 中重复性 T/RH 测量 | 最大 4.5 ms；返回 6 字节 |
| `0xE0` | 低重复性 T/RH 测量 | 最大 1.6 ms；返回 6 字节 |
| `0x89` | 读取唯一序列号 | 返回两个 16 位字及各自 CRC，共 6 字节 |
| `0x94` | 软件复位 | 最大 1 ms 后回到 Idle |
| `0x39`/`0x32` | 200 mW heater，1 s/0.1 s | heater 后执行高重复性测量并返回 6 字节 |
| `0x2F`/`0x24` | 110 mW heater，1 s/0.1 s | 同上 |
| `0x1E`/`0x15` | 20 mW heater，1 s/0.1 s | 同上 |

依据：[SHT4x Datasheet §3.1、§4.5](https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf)。传感器不使用 clock stretching；测量未完成时读请求会 NACK。数据成功读取一次后即从传感器缓冲中删除，因此主机应按命令规定的最长转换时间等待，再一次读完 6 字节。

响应格式固定为：

```text
T_MSB, T_LSB, CRC(T), RH_MSB, RH_LSB, CRC(RH)
```

CRC-8 参数为多项式 `0x31`、初值 `0xFF`、不反射、final XOR `0x00`，每个 16 位字分别计算。已知校验向量为 `CRC(0xBE, 0xEF) = 0x92`。[SHT4x Datasheet §4.3、§4.4](https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf)

原始值换算：

```text
T(°C) = -45 + 175 × ST / 65535
RH(%RH) = -6 + 125 × SRH / 65535
```

RH 换算可能略小于 0 或大于 100。用于常规 UI/控制时裁剪到 0–100 %RH；科研分析若保留未裁剪值，应在字段名中明确。依据：[SHT4x Datasheet §4.6](https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf)。

## Heater 的正确用途

heater 的主要用途是去除传感器表面的凝露/喷溅水，或在长期高湿环境中通过短脉冲缓解 creep。它会自动在 0.1 s 或 1 s 后关闭；没有单独的 heater-off 命令。

| 档位 | 3.3 V 下典型功率 | 供电电流典型/最大 |
|---|---:|---:|
| 高 | 200 mW | 60/100 mA |
| 中 | 110 mW | 33/55 mA |
| 低 | 20 mW | 6/10 mA |

约束：heater 总占空比必须小于 10%；heater 工作时 RH/T 精度规格不成立，读到的温度也受自热和机械应力影响；芯片温度不得超过 125 °C。未知 QWIIC 模块的稳压器、走线和连接器能否承受最高档瞬态电流尚未证实，默认不启用高档 heater。[SHT4x Datasheet §4.9](https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf)

## 与 SGP41/ENS160 的关系

- SHT40 是温湿度传感器，可以向 SGP41 原始气体测量命令提供温湿度补偿参数，也可以为 ENS160 的 `TEMP_IN`/`RH_IN` 提供真实环境值。
- SHT40 自己不计算 VOC Index、NOx Index、AQI、TVOC 或 eCO2。
- SHT40 heater 期间的读数带有明显自热，不应用作 SGP41/ENS160 的环境补偿输入。应使用 heater 关闭后的独立正常测量，并为模块热稳定预留时间。
- 若 SHT40 与 SGP41/ENS160 共用 I²C，总线调度必须考虑 SHT40 转换期间的 NACK、每个器件地址和各自上拉的并联效果。

## ESP-IDF 6.0.1 集成边界

Sensirion 提供 BSD-3-Clause 的纯 C [embedded-i2c-sht4x](https://github.com/Sensirion/embedded-i2c-sht4x) 驱动；检索时 master 最新提交为 `1b6d714`（2025-11-05）。它比 Arduino 封装更适合本工程，但 `sensirion_i2c_hal.c` 是待实现的平台抽象层，不是即插即用 ESP-IDF 组件。

推荐集成方式：

1. 复用官方 `sht4x_i2c.c/.h`、`sensirion_common.*` 和 CRC/帧处理代码，保留 BSD-3-Clause 许可声明。
2. 将 Sensirion HAL 适配到本工程唯一的 `i2c_master_bus_handle_t`，设备地址使用实测 `0x44`/`0x45`/`0x46`。
3. 使用 `XIAO_I2C_SDA`/`XIAO_I2C_SCL`，默认 100 kHz。SHT40 芯片虽支持 Fast Mode Plus，但本工程 ESP32-S3 官方 I²C 上限为 400 kHz，未知模块也未验证 1 MHz。
4. 命令写入和稍后读取是两个事务；不要用一次立即执行的 transmit-receive 省略转换等待。等待后若 NACK，只做有界重试。
5. 把 CRC 错误、NACK/timeout、超范围换算和复位分别计数；CRC 错误时整组样本无效。
6. 高档 heater 只有在电源能力和实物模块电路验证后才能开放，并在上层标记该次读数为 heater sample，不参与气体补偿。

ESP-IDF API 与电气建议依据：[ESP-IDF 6.0.1 ESP32-S3 I²C 文档](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html)。

## 模块待核对清单

在拿到 QWIIC SHT40 实物/商品链接或原理图之前，下列信息均为未知，不应写成确定参数：

- 完整芯片订货型号及实际地址；
- 模块电源输入范围，是否存在 LDO、MOSFET 电平转换或反接保护；
- SDA/SCL 上拉电压与阻值，是否可切断；
- Qwiic 类连接器针序是否严格兼容既有生态；
- 是否同时引出焊盘、是否有地址选择电路；
- heater 最高档时模块供电压降和连接器温升；
- 传感器开口、PCB 热源和外壳气流对测量偏差的影响。

最小实物验收：断电测 SDA/SCL 到电源的上拉阻值；核对电源网络；3.3 V 限流上电；扫描 `0x44`–`0x46`；读取序列号两次并校验 CRC；对比室温/湿度参考；再决定是否测试低档 heater。

## 一手来源清单

所有来源均在 2026-07-19 检索：

1. [Sensirion SHT4x Datasheet v7.1](https://sensirion.com/media/documents/33FD6951/67EB9032/HT_DS_Datasheet_SHT4x_5.pdf)：2025-03，芯片规格、地址、命令、CRC、换算和 heater 约束。Sensirion 产品页把下载月份显示为 2025-04；版本日期以 PDF 内文为准。
2. [Sensirion SHT40 产品页](https://sensirion.com/products/catalog/SHT40)：当前产品定位、典型精度和官方最新下载入口。
3. [Sensirion embedded-i2c-sht4x](https://github.com/Sensirion/embedded-i2c-sht4x)：当前生成式嵌入式 C 驱动及 HAL 边界。
4. [Sensirion embedded-sht](https://github.com/Sensirion/embedded-sht)：数据表仍指向的旧版聚合仓库；新开发优先使用型号化的 `embedded-i2c-sht4x`。
5. [ESP-IDF 6.0.1 ESP32-S3 I²C 文档](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html)：当前工程基线的 I²C master API、地址和上拉规则。
