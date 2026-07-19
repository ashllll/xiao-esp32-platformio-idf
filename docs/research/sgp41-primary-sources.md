# GY-SGP41 一手资料核对

检索日期：2026-07-19

`GY-SGP41` 是通用模块名称，无法唯一确定 PCB 厂商和电路版本。本文把 Sensirion SGP41 裸芯片能力作为已证实事实；模块是否含稳压器、电平转换、I²C 上拉以及是否支持 5 V，必须根据实物原理图、丝印或测量结果复核。

## 结论摘要

- SGP41 输出 `SRAW_VOC` 和 `SRAW_NOX` 原始信号，配合 Sensirion Gas Index Algorithm 可得到 1–500 的相对 VOC/NOx Index。它不直接输出 ppm/ppb、TVOC、AQI 或 eCO2。
- 裸芯片 `VDD` 与 heater 电源 `VDDH` 均为 1.7–3.6 V，典型 3.3 V，并应接到同一电源。未知 GY 模块不能据此推断支持 5 V。
- 固定 7 位 I²C 地址为 `0x59`，最高 400 kHz。16 位参数和响应字各带一个 CRC-8，命令本身不带 CRC。
- 上电或执行 heater-off 后必须先做 10 s conditioning：每秒一次，不能超过 10 s。之后以严格、连续的 1 Hz 节拍测量，才能符合官方算法的预期。
- 每次测量都应写入外部温湿度补偿值。默认参数不是“固定 25 °C/50 %RH 补偿”，而是明确关闭补偿。
- 官方嵌入式 C 驱动与 Gas Index Algorithm 是两个独立仓库；它们可移植到 ESP-IDF，但需要接入本工程共享的 I²C HAL 和定时基础。

## 电气与环境边界

以下参数来自 [Sensirion SGP41 Datasheet v1.0](https://sensirion.com/media/documents/5FE8673C/61E96F50/Sensirion_Gas_Sensors_Datasheet_SGP41.pdf)（2021-12），描述裸芯片而不是未知模块的输入端。

| 项目 | SGP41 芯片值 | 备注 |
|---|---:|---|
| VDD、VDDH | 1.7–3.6 V，典型 3.3 V | 两路接同一电源；无独立 VDDIO |
| Idle 电流 | 34 µA 典型，105 µA 最大 | heater 关闭 |
| Conditioning 电流 | 4.2 mA 典型，4.6 mA 最大 | 3.3 V |
| 连续 VOC+NOx 电流 | 3.0 mA 典型，3.4 mA 最大 | 1 Hz |
| I²C 地址 | `0x59` | 固定 7 位地址 |
| I²C 速率 | 最高 400 kHz | Fast-mode |
| 推荐工作环境 | -10–50 °C，0–90 %RH | 非凝露 |
| 绝对工作温度 | -20–55 °C | 不表示全范围算法性能相同 |

发出测量命令后 5 ms 内，VDDH 瞬时电流可比表中值高约 20%；供电去耦和电源裕量应覆盖该瞬态。湿度补偿在绝对湿度 1.5–30 g/m³ 范围内得到额外优化，超出时仍须遵守非凝露与工作范围限制。

对于具体 GY-SGP41 板，应先确认：VIN 与芯片电源之间是否有 LDO、SDA/SCL 是否有电平转换、上拉接向哪个电压、连接器针序，以及是否有额外器件。未确认前推荐整板只接 3.3 V，并确保 XIAO 与模块共地。

## I²C 命令、字节序与 CRC

SGP41 使用 16 位命令，按高字节在前发送。命令的两个字节不附带 CRC；每个 16 位参数或返回字后紧跟 1 字节 CRC。

| 命令 | 数值 | 用途 | 命令后等待时间 |
|---|---:|---|---:|
| Execute conditioning | `0x2612` | 上电/heater-off 后调理 | 典型 45 ms，最大 50 ms |
| Measure raw signals | `0x2619` | 测量 VOC 与 NOx 原始信号 | 典型 45 ms，最大 50 ms |
| Execute self-test | `0x280E` | 内部自检 | 典型 300 ms，最大 320 ms |
| Turn heater off | `0x3615` | 进入 Idle | 无测量结果 |
| Get serial number | `0x3682` | 读取唯一序列号 | 返回 3 个字及 CRC |

CRC-8 参数为：多项式 `0x31`（x⁸+x⁵+x⁴+1）、初值 `0xFF`、不反射、结果异或 `0x00`。数据字 `0xBEEF` 的已知 CRC 为 `0x92`，可作为单元测试向量。生产驱动应拒绝任一 CRC 错误的采样。

I²C general-call 地址 `0x00` 后写 `0x06` 会软复位总线上所有响应 general call 的器件；共享总线中不要把它当作 SGP41 专用复位。优先通过电源控制或明确评估整条总线后再使用。

ESP-IDF 6.0.1 应使用 [`driver/i2c_master.h`](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html)，把 `0x59` 作为未左移的 7 位地址。SDA/SCL 为开漏，正式硬件应核对模块已有上拉并计算并联后的等效阻值。

## Conditioning 与连续测量状态机

上电复位或执行 `0x3615` 后：

1. 每 1 s 发送一次 `0x2612`，连续 10 次，总时长 10 s。
2. Conditioning 阶段使用温湿度补偿参数；该命令只返回 `SRAW_VOC`，这些值不送入正式算法输出。
3. 第 11 秒开始，每 1 s 发送一次 `0x2619`，读取 `SRAW_VOC`、`SRAW_NOX`，分别送入两个算法实例。
4. 持续保持 1 Hz。需要省电时可关闭 heater，但再次启用必须重做完整的 10 s conditioning。

数据表明确要求 conditioning 不得超过 10 s。不能通过无限重复 conditioning 来“预热更充分”。如果应用不使用 Gas Index Algorithm，数据表仍要求丢弃启动初期无效值，并结合响应时间与实际校准解释原始信号。

典型 T63 响应时间为 VOC 小于 10 s、NOx 小于 250 s；气体事件可靠检测通常小于 60 s。但完整规定条件下，SRAW VOC 可需小于 1 h、VOC Index 小于 1.5 h、SRAW NOx 小于 7 天、NOx Index 小于 6 h 才达到所列精度。它们是不同定义的稳定/精度时间，不应合并成一个“预热 X 秒后准确”的宣传值。

## 温湿度补偿

`0x2612` 与 `0x2619` 都接收两个 16 位补偿参数，每个参数后附 CRC：

```text
RH_ticks = RH_percent * 65535 / 100
T_ticks  = (T_celsius + 45) * 65535 / 175
```

输入应限幅在协议范围内，使用同一时刻附近的真实环境值。SHT40 是本工程合适的补偿来源，但需避免 SGP41 heater、ESP32 或显示器自热影响温湿度采样位置。

数据表的默认字为湿度 `0x8000`、温度 `0x6666`。尽管它们数值上约等于 50 %RH 和 25 °C，协议定义这组默认值表示“补偿关闭”；不能写成传感器正在用固定 25 °C/50 %RH 做补偿。

## 输出含义与算法边界

| 层级 | 输出 | 范围 | 正确解释 |
|---|---|---:|---|
| 芯片命令 | `SRAW_VOC` | 0–65,535 ticks | 与气体响应对数相关的原始量，不是浓度 |
| 芯片命令 | `SRAW_NOX` | 0–65,535 ticks | 与 NOx 响应相关的原始量，不是 NO₂ ppm/ppb |
| Gas Index Algorithm | VOC Index | 1–500 | 100 对应过去约 24 h 的典型背景；高于 100 表示更差，低于 100 表示更好 |
| Gas Index Algorithm | NOx Index | 1–500 | 基线为 1，升高表示 NOx 事件；不是法规 AQI |

数据表中的 proxy 测试范围用于验证器件响应，不是可由原始 ticks 直接换算的目标气体浓度。SGP41 是宽带 MOX 气体传感器，不能把输出标成“TVOC ppm”“NO₂ ppm”“真实 AQI”或“eCO2”。需要绝对浓度或法规合规测量时，应选择相应的选择性、经校准传感器和认证方案。

官方 [Gas Index Algorithm](https://github.com/Sensirion/gas-index-algorithm) 为 VOC 与 NOx 分别维护长期自适应状态；实例不能共用。中断采样、改变周期或重启会影响基线，工程应明确持久化策略、冷启动状态和数据有效标志，而不是只记录一个整数。

## ESP-IDF 集成建议

Sensirion 当前维护的 [`embedded-i2c-sgp41`](https://github.com/Sensirion/embedded-i2c-sgp41) 是通用 C 驱动，不是可直接使用的 ESP-IDF component。建议：

1. 把设备访问封装为本工程共享 I²C 总线上的组件，不由每个传感器重复创建总线。
2. 为命令编码、CRC、补偿转换、10 s conditioning 和 1 Hz 调度写单元测试。
3. 将官方 `gas-index-algorithm` 作为独立组件锁定具体版本，VOC/NOx 各建一个状态实例。
4. 对 I²C 错误、CRC 错误、采样超时和非 1 Hz 周期设置显式有效性状态；异常样本不要推进算法。
5. 日志同时保存原始 ticks、Index、补偿温湿度、运行阶段与算法版本，便于排障和重新解释。

旧 [`Sensirion/embedded-sgp`](https://github.com/Sensirion/embedded-sgp) 已于 2024-04-19 归档，不应作为新工程入口。网上提到的 `embedded-gas-index-algorithm` 并不是当前有效仓库名，正确入口是 `Sensirion/gas-index-algorithm`。

## 一手资料索引

- [Sensirion SGP41 产品页](https://sensirion.com/products/catalog/SGP41)
- [Sensirion SGP41 Datasheet v1.0](https://sensirion.com/media/documents/5FE8673C/61E96F50/Sensirion_Gas_Sensors_Datasheet_SGP41.pdf)
- [Sensirion embedded-i2c-sgp41](https://github.com/Sensirion/embedded-i2c-sgp41)
- [Sensirion Gas Index Algorithm](https://github.com/Sensirion/gas-index-algorithm)
- [ESP-IDF 6.0.1 I²C Master Driver](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html)
