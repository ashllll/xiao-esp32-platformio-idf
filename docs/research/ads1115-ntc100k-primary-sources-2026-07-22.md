# ADS1115 + NTC 100K 测温一手资料核对 / Primary-source review

检索日期 / Review date：2026-07-22

本文核对在本仓库 ESP-IDF 6.0.1 与 Seeed Studio XIAO ESP32C3、ESP32S3、ESP32C6 上，通过 Texas Instruments ADS1115 读取 100 kΩ NTC 分压器的实现边界。外部事实只使用 TI、Espressif、Seeed、TDK Electronics（EPCOS）和 Vishay 的一手资料。这里的参数与流程是后续示例的设计依据，不代表任何具体 ADS1115 模块或 NTC 实物已经通过真机测试。

This note establishes the implementation boundaries for reading a 100 kΩ NTC divider through a Texas Instruments ADS1115 on ESP-IDF 6.0.1 and the Seeed Studio XIAO ESP32C3, ESP32S3, and ESP32C6. External claims are based only on first-party material from TI, Espressif, Seeed, TDK Electronics (EPCOS), and Vishay. The proposed parameters and sequence are design inputs for a later example; they do not prove that any particular ADS1115 module or thermistor has passed hardware testing.

## 结论摘要 / Executive summary

- **供电与输入范围 / Supply and input limits**：ADS1115 的工作电源范围是 2.0–5.5 V；在 XIAO 上应把 ADC 及 I²C 上拉都置于 3.3 V 域。PGA 的 `±4.096 V` 只是量程刻度，不会让 3.3 V 供电的器件获得 4.096 V 输入耐压。每个模拟输入仍必须位于 GND 到 VDD 的正常输入范围内，绝对极限为 GND−0.3 V 到 VDD+0.3 V。
- **推荐 ADS1115 初始配置 / Recommended starting configuration**：`ADDR=GND`（7-bit `0x48`）、AIN0 单端、PGA `±4.096 V`（125 µV/LSB）、128 SPS、single-shot、comparator disabled、I²C 100 kHz。对应 Config 值为 `0xC383`。以轮询 OS 位判断转换完成，不依赖刚好 7.8125 ms 的固定延时。
- **推荐分压器 / Recommended divider**：3.3 V → 100 kΩ、1% 固定电阻 → AIN0 → 100 kΩ NTC → GND；在 25 °C 时理想节点约 1.65 V。100 kΩ 固定电阻有利于限制 NTC 自热，但使节点 Thevenin 源阻抗在 25 °C 达 50 kΩ，ADS1115 的有限且随 PGA 改变的输入阻抗会造成不可忽略的系统误差。高准确度设计应做实物校准，或用低偏置缓冲器；不能把“16 bit”直接等同于 16 bit 系统准确度。
- **“NTC 100K”不是完整型号 / “100K NTC” is not a complete specification**：它通常只说明 `R25≈100 kΩ`。Beta 值、Beta 的两个温度端点、R25/Beta 公差、R/T 曲线、封装、耗散系数和温区均可能不同。示例若要有可验证的默认器件，可指定 TDK `B57541G1104F000`：R25=100 kΩ ±1%，B25/85=4072 K，B25/100=4092 K；不能把这些参数套到未知的“100K B3950”或其他料号。
- **Beta 公式只是近似 / The Beta equation is an approximation**：单 Beta 公式适合教学、较窄温区或宽松误差目标。Vishay 明确指出它在 0–100 °C 即使条件理想也不保证 ±1 °C。更严谨的实现应使用目标料号的 R/T 表插值，或由至少三个点建立 Steinhart–Hart 系数。
- **供电值也是测量量 / Excitation voltage is part of the measurement**：ADS1115 使用内部参考，不会自动消除 3.3 V 分压激励的误差。电阻换算必须使用实际 `Vexc`，或通过系统校准/额外通道建立比值测量；把标称 3.300 V 写死会把稳压误差直接带入温度结果。

## 1. ADS1115 已核对规格 / Verified ADS1115 facts

主来源：[TI ADS1113/ADS1114/ADS1115 Datasheet, SBAS444E, revised December 2024](https://www.ti.com/lit/ds/symlink/ads1115.pdf)。

| 项目 / Item | 数据表结论 / Datasheet result | 示例含义 / Implication |
|---|---|---|
| 工作电源 / Operating supply | 2.0–5.5 V | XIAO 示例统一用 3.3 V |
| 电源绝对最大值 / Absolute maximum VDD | −0.3–7 V | 绝对最大额定值不是正常工作范围 |
| 模拟输入绝对最大值 / Absolute maximum AINx | GND−0.3 V 至 VDD+0.3 V | 任一 AIN 都不能因 PGA 量程较大而越过电源轨 |
| 正常模拟输入 / Functional analog input | 每个 AIN 从 GND 到 VDD；单端上限是 `min(VDD,+FS)` | 3.3 V 供电时不能取得 `±4.096 V` 的完整正向码范围 |
| 分辨率与格式 / Resolution and format | 16 bit，二进制补码 | 单端只使用 `0x0000`–`0x7FFF` 正码半区；近 0 V 可因 offset 出现负码 |
| PGA FSR / PGA ranges | ±6.144、±4.096、±2.048、±1.024、±0.512、±0.256 V | FSR 是差分刻度，不是输入保护范围 |
| LSB | 187.5、125、62.5、31.25、15.625、7.8125 µV | 本方案 ±4.096 V 对应 125 µV/LSB |
| 数据率 / Data rates | 8、16、32、64、128、250、475、860 SPS；变化 ±10% | 128 SPS 标称 7.8125 ms，完成时间需留公差并轮询 OS |
| 电源启动 / Single-shot start | 写 Config 的 OS=1 后约 25 µs 上电并开始一次转换；完成后再次掉电 | 写 `0xC383` 启动一次采样 |
| I²C 地址 / Addresses | ADDR 接 GND/VDD/SDA/SCL → `0x48/0x49/0x4A/0x4B` | 默认 ADDR 接 GND；文档与代码统一使用 7-bit 地址 |
| I²C 速度 / I²C speed | 100 kHz、400 kHz、3.4 MHz 模式 | 教学示例选 100 kHz，兼顾模块接线和上拉 |
| 去耦与上拉 / Decoupling and pull-ups | VDD 附近 0.1 µF；SDA/SCL 需要上拉，典型 1–10 kΩ | 模块若已有上拉，要确认其电压和等效阻值 |

### 1.1 PGA 不等于输入耐压 / PGA range is not input tolerance

TI 明确区分差分 FSR 与每个输入脚的绝对电压。即使 Config 选择 `±4.096 V` 或 `±6.144 V`，3.3 V 供电时每个 AIN 仍不能正常测量高于 VDD 的电压，更不能依靠内部 ESD 二极管持续钳位。未知模块若以 5 V 供电且把 SDA/SCL 上拉到 5 V，也不应直接接 XIAO；尽管 ADS1115 数字输入绝对最大值可到 5.5 V，ESP32 GPIO 并非 5 V tolerant。

TI distinguishes the differential full-scale range from the absolute voltage on each input pin. Selecting `±4.096 V` or `±6.144 V` does not permit an AIN pin to exceed VDD in normal operation. Likewise, a breakout powered from 5 V must not expose 5 V I²C pull-ups to the XIAO.

### 1.2 输入阻抗与高源阻抗误差 / Input impedance and high-source-impedance error

ADS1115 前端是开关电容 ΔΣ 调制器，不是理想无限输入阻抗电压表。数据表给出的典型输入阻抗随 PGA 变化：

| FSR | 典型共模输入阻抗 / Common-mode | 典型差分输入阻抗 / Differential |
|---:|---:|---:|
| ±6.144 V | 10 MΩ | 22 MΩ |
| ±4.096 V | 6 MΩ | 15 MΩ |
| ±2.048 V | 6 MΩ | 4.9 MΩ |
| ±1.024 V | 3 MΩ | 2.4 MΩ |
| ±0.512/±0.256 V | 100 MΩ | 710 kΩ |

TI 明确要求在信号源输出阻抗较高时考虑测量偏差，必要时加缓冲器。100 kΩ/100 kΩ 分压器在 25 °C 的 Thevenin 阻抗为 50 kΩ；它与 MΩ 级输入阻抗相比不再是“零误差”。由于单端输入同时受共模与差分开关网络影响，不能只拿表中某一个阻抗值做精密直流修正。对目标硬件应使用精密电阻/温度参考点做系统校准；需要更高准确度时加入低输入偏置、轨到轨缓冲器，并重新计入缓冲器 offset、gain、noise 和输出稳定性。

The 100 kΩ/100 kΩ divider has a 50 kΩ Thevenin resistance at 25 °C. That is not negligible relative to the ADS1115's MΩ-scale, PGA-dependent switched-capacitor input impedance. A single table value is not a complete precision model for single-ended loading; calibrate the assembled system or buffer it when the error budget requires this.

### 1.3 单次转换寄存器流程 / Single-shot register sequence

TI 定义的寄存器指针为 Conversion=`0x00`、Config=`0x01`、Lo_thresh=`0x02`、Hi_thresh=`0x03`。推荐流程：

1. 向 `0x48` 写三个字节：`0x01, 0xC3, 0x83`。
2. 周期性以 repeated START 读 Config：先发送指针 `0x01`，再读两个字节；OS(bit 15)=0 表示转换中，=1 表示空闲/完成。
3. 在带上限的超时内等待 OS=1。128 SPS 数据率存在 ±10% 变化，建议超时至少覆盖最慢一帧并留 I²C/任务调度余量，而非恰好等待 8 ms。
4. 发送指针 `0x00` 并 repeated START 读两个字节；高字节在前，合并为有符号 `int16_t`。
5. `volts = code × 125e-6`。单端输入理论上非负，但接近 0 V 时 offset 可得到小负码，应用应识别而非转成巨大的无符号数。

`0xC383` 的字段是 OS=1、MUX=`100`（AIN0−GND）、PGA=`001`（±4.096 V）、MODE=1（single-shot）、DR=`100`（128 SPS）、COMP_QUE=`11`（comparator disabled），其他 comparator 位为 0。

In ESP-IDF 6.0.1, register reads map naturally to `i2c_master_transmit_receive()`, which performs the write and read without inserting STOP between them. Bus setup uses `driver/i2c_master.h`, `i2c_new_master_bus()`, and `i2c_master_bus_add_device()`; the legacy `driver/i2c.h` API should not be used for a new example.

主来源 / Primary source：[ESP-IDF 6.0.1 I²C master driver](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32/api-reference/peripherals/i2c.html)。

## 2. XIAO 三板接线 / Wiring across the three XIAO boards

三块板都使用 XIAO 排针 D4/D5 作为仓库默认 I²C SDA/SCL，但底层 GPIO 不同：

| 信号 | XIAO ESP32C3 | XIAO ESP32S3 | XIAO ESP32C6 |
|---|---:|---:|---:|
| SDA / D4 | GPIO6 | GPIO5 | GPIO22 |
| SCL / D5 | GPIO7 | GPIO6 | GPIO23 |

代码应使用仓库的 `XIAO_I2C_SDA` 与 `XIAO_I2C_SCL`，不要在跨板组件中写死裸 GPIO。

推荐裸芯片/已核对模块接线：

```text
XIAO 3V3 ───────── ADS1115 VDD
XIAO GND ───────── ADS1115 GND ───────── NTC lower end
XIAO D4/SDA ────── ADS1115 SDA
XIAO D5/SCL ────── ADS1115 SCL
ADS1115 ADDR ───── GND                 (7-bit address 0x48)

XIAO 3V3 ── R_FIXED 100 kΩ, 1% ──+── ADS1115 AIN0
                                   |
                              NTC 100 kΩ
                                   |
                                  GND
```

ADS1115 VDD 处放 0.1 µF 去耦。SDA/SCL 上拉应接 3.3 V；若 breakout 已装上拉，要断电测量其连接目标和阻值。Seeed 官方引脚依据：[C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#pin-map)、[S3](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#hardware-overview)、[C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#pin-map)。

## 3. NTC 100K 参数边界 / What “NTC 100K” does and does not specify

### 3.1 一个真实原厂料号 / One concrete first-party part

[TDK Electronics B57541G1 datasheet, June 2026](https://www.tdk-electronics.tdk.com/inf/50/db/ntc/NTC_Glass_enc_sensors_G1541_coated.pdf) 给出的 100 kΩ 版本为 `B57541G1104+000`，其中 `+` 表示 R25 公差代码：F=±1%、G=±2%、H=±3%。以 `B57541G1104F000` 为例：

| 参数 / Parameter | 值 / Value |
|---|---:|
| R25 | 100 kΩ ±1% |
| R/T characteristic | 8304 |
| B25/85 | 4072 K |
| B0/100 | 4036 K |
| B25/100 | 4092 K ±1% |
| 工作温区 / Temperature category | −55–250 °C（仍需遵守功率和安装限制） |
| 25 °C 最大功率 / Max power at 25 °C | 18 mW |
| 空气中耗散系数 / Dissipation factor in air | 约 0.45 mW/K |
| 空气中热时间常数 / Thermal time constant in air | 约 4.5 s |

同一个真实 100 kΩ NTC 已经同时列出 B25/85=4072 K 与 B25/100=4092 K。这直接证明 Beta 必须连同端点使用；“100K 3950”不是所有 100 kΩ NTC 的通用属性。

The same real 100 kΩ part specifies B25/85=4072 K and B25/100=4092 K. Beta therefore belongs with its endpoint temperatures; neither “100K” nor “B3950” can be silently generalized to every 100 kΩ thermistor.

### 3.2 分压与 Beta 公式 / Divider and Beta equations

对本文“固定电阻在上、NTC 接地”的拓扑：

```text
Vnode = Vexc × Rntc / (Rfixed + Rntc)
Rntc = Rfixed × Vnode / (Vexc - Vnode)
```

单 Beta 近似为：

```text
T_kelvin = 1 / (1 / T0_kelvin + ln(Rntc / R0) / Beta)
T_celsius = T_kelvin - 273.15
```

`ln` 必须是自然对数，温度必须用 Kelvin；通常 R0=100000 Ω、T0=298.15 K。若 NTC 与固定电阻上下互换，电阻反算式也必须更换，不能只在软件中把温度符号取反。

Vishay 的一手技术说明指出，Beta 方程只在较窄温区适合宽松公差使用，在 0–100 °C 即使条件良好也不能达到 ±1 °C；三点 Steinhart–Hart 或目标料号 R/T 表能提供更好的拟合。[Vishay “Selecting NTC Thermistors”](https://www.vishay.com/docs/33001/seltherm.pdf) 同时说明 Steinhart–Hart 至少需要三个校准点。

### 3.3 自热、误差与校准 / Self-heating, error, and calibration

25 °C、3.3 V、100 kΩ + 100 kΩ 时，NTC 理想耗散约：

```text
I = 3.3 V / 200 kΩ = 16.5 µA
Pntc = I² × 100 kΩ ≈ 27.2 µW
ΔT_self ≈ 0.0272 mW / 0.45 mW/K ≈ 0.06 K  (still air, approximate)
```

这只是按 TDK 料号空气中典型耗散系数计算的稳态估算。实际 PCB 铜箔、封装、灌封、气流、液体接触和安装压力都会改变耗散与热时间常数。TDK 数据表的 AEC-Q200 operational-life 条件也明确限制测试自热不超过 0.2 K，说明“测温电流很小”仍需量化。

误差预算至少包含：

- NTC R25 公差、Beta/曲线公差以及 Beta 近似的模型误差；
- 固定电阻公差和温漂；
- 实际 Vexc 与软件值之差；
- ADS1115 gain、offset、noise、有限输入阻抗及其温漂；
- 分压器和 ADC 的自热、PCB/空气热梯度、响应时间；
- 焊接、引线、电缆漏电、潮气/污染，尤其在低温时 NTC 阻值可达 MΩ；
- 量程端点的除零/饱和：`Vnode≈0` 或 `Vnode≈Vexc` 时电阻反算极不稳定。

建议至少用两个或三个已知温度点做整机校准，并记录原始 ADC code、Vnode、Vexc、Rfixed、NTC 料号/批次、换算模型和 calibration ID。冰水点只覆盖一个点，且实际操作必须处理水纯度、相平衡、探头浸没与防水；不应单独宣称全温区准确度。

Vishay 对自热的定义与选择建议见 [How to select an NTC Thermistor](https://www.vishay.com/doc/?33001=)：耗散系数是使热敏电阻高于环境 1 °C 所需的功率，允许的自热应只占总测量误差预算的一部分。

## 4. 建议的默认示例参数 / Recommended example defaults

| 配置项 | 建议默认值 | 原因与限制 |
|---|---:|---|
| ADS1115 supply / logic | 3.3 V | 与 XIAO GPIO 同电压域 |
| ADS1115 address | `0x48` | ADDR 接 GND |
| I²C | 100 kHz | 三板通用的保守起点 |
| Input | AIN0 single-ended | MUX=`100` |
| PGA / LSB | ±4.096 V / 125 µV | 覆盖 0–3.3 V 节点；FSR 不改变输入绝对限制 |
| Mode / DR | single-shot / 128 SPS | 温度变化慢，轮询 OS 完成 |
| Config word | `0xC383` | comparator disabled |
| Rfixed | 100 kΩ, 1%, low-TCR | 控制自热；接受并校准较高源阻抗误差 |
| Divider | Rfixed high side, NTC low side | 25 °C 约 1.65 V，升温时电压下降 |
| Example NTC | TDK `B57541G1104F000` | 真实可追溯料号，不以“通用 100K”替代 |
| R0 / T0 | 100000 Ω / 298.15 K | 与该料号 R25 定义一致 |
| Beta demo | B25/85=4072 K | 仅用于相应温区的简化演示；生产实现优先 R/T 表或 Steinhart–Hart |
| Vexc | 实测/校准值，不默认等于精确 3.300 V | ADS1115 内部参考不提供自动比值抵消 |

若目标是兼容用户手中的未知“NTC100K B3950”，代码应把 R25、Beta、Beta 端点或 Steinhart–Hart 系数全部做成显式配置，并把 `3950 K` 标为用户提供的器件参数，不能作为已核验硬件事实。

For an unknown “NTC100K B3950”, expose R25, Beta and its endpoints—or Steinhart–Hart coefficients—as explicit configuration. Treat 3950 K as user-supplied part data, not as a verified universal default.

## 5. 实施前必须关闭的风险 / Risks to close before implementation

1. **模块身份 / Breakout identity**：确认是 ADS1115 而不是 ADS1015，读取/写回 Config 并核对行为；检查模块原理图、I²C 上拉电压、ADDR 焊盘和是否有额外串联电阻。
2. **NTC 料号 / Thermistor identity**：取得制造商料号和数据表。未知料号只能演示趋势，不能承诺温度准确度。
3. **Vexc / ratiometry**：决定是实测 3.3 V、用额外 ADC 通道估计供电，还是通过已知温度点把供电和 ADC loading 一并校准。直接写死 3.3 V 仅适合演示。
4. **输入 loading**：对 100 kΩ 分压器做 SPICE/台架对比，或加入缓冲并重新做误差预算；不要用 ADC 分辨率代替系统准确度。
5. **异常检测 / Fault detection**：开路会趋近 Vexc，短路会趋近 0 V；在接近两端时报告 open/short/saturated，而不是输出极端但看似有效的温度。
6. **真机验证 / Hardware validation**：在 C3/S3/C6 至少分别完成 I²C 探测、Config 回读、已知电阻替代 NTC 的电阻反算、两个以上温度点和长期稳定性测试。

## 一手来源清单 / First-party source register

以下来源均于 2026-07-22 检索 / All sources retrieved 2026-07-22:

1. [Texas Instruments ADS1113/ADS1114/ADS1115 Datasheet, SBAS444E](https://www.ti.com/lit/ds/symlink/ads1115.pdf)：供电、输入限制、PGA/LSB、输入阻抗、数据率、I²C 地址、single-shot、寄存器和典型连接。
2. [ESP-IDF 6.0.1 I²C Programming Guide](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32/api-reference/peripherals/i2c.html)：新 master bus/device API 与 repeated-START register read。
3. Seeed Studio XIAO pin maps：[ESP32C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#pin-map)、[ESP32S3](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#hardware-overview)、[ESP32C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#pin-map)：D4/D5 与实际 GPIO 映射。
4. [TDK Electronics B57541G1 Datasheet, June 2026](https://www.tdk-electronics.tdk.com/inf/50/db/ntc/NTC_Glass_enc_sensors_G1541_coated.pdf)：真实 100 kΩ 料号、R25/Beta/R-T、耗散系数、热时间常数和自热测试限制。
5. [Vishay Selecting NTC Thermistors](https://www.vishay.com/docs/33001/seltherm.pdf)：Beta 与 Steinhart–Hart 的适用边界。
6. [Vishay How to select an NTC Thermistor](https://www.vishay.com/doc/?33001=)：耗散系数、自热和误差预算定义。

## 证据边界 / Evidence boundary

这些资料能证明芯片与指定热敏电阻的设计规格、公式边界和 API 用法；不能证明淘宝/无品牌 ADS1115 模块的芯片真伪与电路、用户手中 NTC 的 R25/Beta、XIAO 3.3 V 的实际值，或任何一块实物的最终温度准确度。必须在代码完成后用已知电阻、可追溯温度参考和三块目标板进行真机验收。

These sources establish component specifications, model limits, and API semantics. They do not establish the authenticity or schematic of an unbranded breakout, the R25/Beta of an unknown thermistor, the actual XIAO 3.3 V rail, or the final accuracy of an assembled unit. Those require bench validation with known resistors, temperature references, and the three target boards.
