# DFRobot Fermion ENS160（SEN0515）一手资料核对

检索日期：2026-07-19

本文是把 SEN0515 接入本工程前的证据笔记。参数以具体对象为边界：DFRobot 的 SEN0515 成品模块与 ScioSense 的 ENS160 裸芯片不是同一电气层级。原厂 Production 数据表、DFRobot 产品页与实际库源码依次用于确认芯片能力、模块接口和软件行为；营销性描述不用于推导精度或安全用途。

## 结论摘要

- SEN0515 模块由 `3V3` 供电；不要把裸 ENS160 的 `VDD=1.71–1.98 V` 写成模块输入范围，也不要因裸芯片 `VDDIO` 最高 3.6 V 就推断模块可以接 5 V。
- Fermion 版本默认 7 位 I²C 地址是 `0x53`，将模块 `SDO` 接 GND 后为 `0x52`。地址在上电时采样，修改 SDO 后应重新上电。
- 输出为 AQI-UBA（1–5）、TVOC（0–65,000 ppb）和 eCO2（400–65,000 ppm）。eCO2 是根据 VOC 与氢响应推算的“等效 CO2”，不是直接 CO2 浓度测量。
- 不应把“预热 3 分钟”表述为达到最终准确度：还要区分生命周期首次启动约 1 小时、连续运行 24 小时后首次启动状态才写入非易失存储器，以及首次 48 小时内传感电阻变化仍较大。
- 温湿度补偿数据来自外部温湿度传感器；ENS160 本身不直接测量环境温湿度。补偿数据应定期写入 `TEMP_IN`/`RH_IN`，而不是长期固定为示例中的 25 °C、50 %RH。
- 本工程是原生 ESP-IDF，不含 Arduino framework。DFRobot Arduino 库依赖 `Arduino.h`、`Wire.h`、`SPI.h`，不能直接加入当前工程；应按其寄存器行为写原生 ESP-IDF 组件，或显式改变工程架构后使用 Arduino as an ESP-IDF component。

## 模块规格与引脚

DFRobot 中文 Wiki 对 SEN0515 模块给出的规格如下。这里的 `3.3 V` 和 `29 mA` 是模块使用入口的资料；`29 mA` 与芯片 Standard 模式典型电流一致，但不能据此忽略芯片切换到 Standard 时小于 5 ms 的 79 mA 峰值。电源设计应留出瞬态裕量，并在实物上测量供电压降。

| 项目 | SEN0515 模块信息 | 依据 |
|---|---:|---|
| 供电 | 3.3 V | [DFRobot 中文 Wiki：技术规格](https://wiki.dfrobot.com.cn/_SKU_SEN0515_Fermion_ENS160_Air_Quality_Sensor_) |
| 工作电流 | 29 mA | 同上 |
| 接口 | I²C 或 SPI | 同上 |
| I²C 地址 | `0x53` 默认；SDO 接 GND 后 `0x52` | [DFRobot 中文 Wiki：技术规格与示例](https://wiki.dfrobot.com.cn/_SKU_SEN0515_Fermion_ENS160_Air_Quality_Sensor_) |
| 标称工作环境 | -40–85 °C；5–95 %RH | 同上；湿度必须非凝露的限制由 ENS160 数据表明确 |
| 模块尺寸 | 20 × 15 mm | 同上 |
| 安装孔 | 孔距 15 mm；孔径 2 mm | 同上 |

SEN0515 的排针丝印和用途：

| 丝印 | 用途 | I²C 接法 |
|---|---|---|
| `3V3` | 模块 3.3 V 电源 | 接 XIAO `3V3` |
| `GND` | 地 | 与 XIAO 共地 |
| `SCL` | I²C 时钟 | 接 XIAO D5/SCL |
| `SDA` | I²C 数据 | 接 XIAO D4/SDA |
| `SCK` | SPI 时钟 | I²C 模式不用 |
| `SDI` | SPI 主机输出/模块输入 | I²C 模式不用 |
| `SDO` | SPI 主机输入/模块输出；I²C 地址选择 | 悬空/高为 `0x53`，接 GND 为 `0x52` |
| `CS` | SPI 片选 | I²C 模式保持非选中状态；模块电路已决定其默认接口行为时不要擅自改接 |
| `INT` | 数据就绪中断 | 可选 GPIO；也可轮询状态寄存器 |

模块引脚表来源：[DFRobot 中文 Wiki：引脚说明](https://wiki.dfrobot.com.cn/_SKU_SEN0515_Fermion_ENS160_Air_Quality_Sensor_)。裸芯片的复用关系和地址采样机制见 [ENS160 Datasheet v1.3 §2、§14](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)。

### 裸芯片参数边界

以下参数只描述 SEN0515 板上的 ENS160 芯片，不是模块外部供电规格：

| 参数 | ENS160 裸芯片 | 数据表位置 |
|---|---:|---|
| VDD | 1.71–1.98 V，典型 1.8 V | §4 Table 3 |
| VDDIO | 1.71–3.6 V | §4 Table 3 |
| Deep Sleep 平均电流 | 0.01 mA 典型；不测气体 | §4 Table 3 |
| Idle 平均电流 | 2 mA 典型、2.5 mA 最大；不测气体 | §4 Table 3 |
| Standard 平均电流 | 29 mA 典型 | §4 Table 3 |
| Standard 启动峰值 | 79 mA、持续小于 5 ms | §4 Table 3 |
| 工作电气范围 | -40–85 °C、5–95 %RH，非凝露 | §3 Table 2 |
| 推荐气敏环境 | -5–60 °C、20–80 %RH，非凝露 | §12 |

SDA 是开漏信号。正式硬件应确认模块上已有上拉及其阻值；多个 I²C 模块并联时要计算等效上拉，不能盲目重复增加。ScioSense 的裸芯片参考电路给出 100 kHz 场景的 4.7 kΩ 示例，但实际值取决于总线电容和线长。ESP-IDF 也明确指出内部上拉不适合高速总线，建议使用合适的外部上拉。[ENS160 Datasheet §4、§17.1](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)，[ESP-IDF 6.0.1 I²C 文档](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html)

## 输出指标的正确含义

| 输出 | 范围/分辨率 | 读取位置 | 解释边界 |
|---|---|---|---|
| AQI-UBA | 1–5，步进 1 | `DATA_AQI` `0x21` | 德国 UBA 的五级 TVOC 空气质量指标 |
| TVOC | 0–65,000 ppb，1 ppb | `DATA_TVOC` `0x22`，2 字节 | 宽带 MOX 算法输出，不是实验室色谱分析 |
| eCO2 | 400–65,000 ppm，1 ppm | `DATA_ECO2` `0x24`，2 字节 | VOC 与氢响应推算的等效值；不可当成 NDIR 真 CO2 |

范围和寄存器来自 [ENS160 Datasheet §5 Table 4、§16.2](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)，DFRobot 的 `getAQI()`、`getTVOC()`、`getECO2()` 只是读取这些寄存器：[DFRobot_ENS160.h](https://github.com/DFRobot/DFRobot_ENS160/blob/main/DFRobot_ENS160.h)、[DFRobot_ENS160.cpp](https://github.com/DFRobot/DFRobot_ENS160/blob/main/DFRobot_ENS160.cpp)。

数据表对氢浓度示例给出的典型误差小于测量值的 12%，测试条件包含气候箱、至少 24 小时预处理、25 °C/50 %RH 和特定校准；它不是对 TVOC、eCO2 或任意实际混合气体的统一精度保证。ENS160 对多种气体具有宽带响应，多数单气体结果需要原始电阻信号与目标气体校准，不应把它宣传为即插即用的选择性单气体浓度传感器。[ENS160 Datasheet §6、§9](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)

该器件明确不用于安全关键或生命保护应用。因此它不能替代合规的烟雾、燃气泄漏、CO 或消防报警器。[ENS160 Datasheet §3、§12](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)

## 启动、预热与数据有效性

必须同时处理下列四个时间尺度：

1. 每次上电或较长 Idle 后，典型预热时间为 3 分钟；此时 `VALIDITY=1`。
2. 传感器生命周期首次上电后，约 1 小时才得到“可合理使用”的 AQI/TVOC/eCO2；此时 `VALIDITY=2`。
3. 首次启动完成状态只有在连续运行 24 小时后才保存到非易失存储器；提前断电会导致下次再次进入 Initial Start-up。
4. 首次上电后的前 48 小时传感电阻变化仍最明显，早期精度可能略低。

依据：[ENS160 Datasheet §10、§11 Table 10、§12 脚注](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)。DFRobot Wiki 也列出 0=正常、1=预热、2=首次启动，并在官方示例中调用 `getENS160Status()`：[DFRobot 中文 Wiki：轮询示例](https://wiki.dfrobot.com.cn/_SKU_SEN0515_Fermion_ENS160_Air_Quality_Sensor_)。

应用程序至少应记录并暴露状态，不要在 `VALIDITY` 为 1、2、3 时把读数标成“已校准正常值”。`VALIDITY=3` 表示无有效输出；还应检查 `DEVICE_STATUS` 的错误位，而不是只看 data-ready。

## 运行模式与关键寄存器

ENS160 上电默认进入 Idle；它能通信但不测气体。主动测量前写 `OPMODE=0x02`。ENS160 Production 数据表只定义以下模式：

| OPMODE | 含义 |
|---:|---|
| `0x00` | Deep Sleep，低功耗待机，不测气体 |
| `0x01` | Idle，用于配置，不测气体 |
| `0x02` | Standard，主动气体测量 |
| `0xF0` | Reset |

ScioSense 的 ENS16x 家族资料会出现 `0x03`/`0x04` 低功耗测量模式，但 2025 通信应用说明明确把它们标为 ENS161-only。不要据此声称 SEN0515/ENS160 支持 150 µA 的低功耗测量。[ENS160 Datasheet §15、§16.2.2](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)，[ENS16x Fundamental Communication Application Note](https://www.sciosense.com/wp-content/uploads/2025/08/ENS16x-Application-Note-Fundamental-Communication.pdf)

原生组件实现时需要的最小寄存器集合：

| 地址 | 名称 | 说明 |
|---:|---|---|
| `0x00` | `PART_ID` | 2 字节 little-endian，应为 `0x0160`（线序 `0x60, 0x01`） |
| `0x10` | `OPMODE` | 运行模式 |
| `0x11` | `CONFIG` | INT 极性、驱动方式和数据就绪中断 |
| `0x13` | `TEMP_IN` | 2 字节外部温度补偿输入 |
| `0x15` | `RH_IN` | 2 字节外部相对湿度补偿输入 |
| `0x20` | `DEVICE_STATUS` | 运行、错误、有效性和新数据标志 |
| `0x21` | `DATA_AQI` | AQI-UBA |
| `0x22` | `DATA_TVOC` | 2 字节 TVOC |
| `0x24` | `DATA_ECO2` | 2 字节 eCO2 |
| `0x30` / `0x32` | `DATA_T` / `DATA_RH` | 算法实际采用的补偿值回读，不是片上传感器测量 |

寄存器为 little-endian。`NEWDAT` 会在首次读取 `DATA_x` 时清除；若一次采集需要多个输出，宜利用地址自动递增一次连续读取以获得同一批数据。轮询和 INT 都可用；INT 的常见配置 `0x23` 是低有效、开漏、DATA 新数据中断。[ENS160 Datasheet §14.1、§16](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)

## 温湿度补偿

ENS160 的补偿算法依赖外部温湿度传感器。环境湿度超出 20–80 %RH 时影响尤其明显；官方建议定期更新内部输入。补偿默认作用于处理后的 AQI、TVOC、eCO2 等输出，但不作用于原始传感器信号。[ENS160 Datasheet §8.2](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)

写入格式：

- `TEMP_IN = (温度 °C + 273.15) × 64`，16 位，先 LSB 后 MSB；25 °C 示例为 `0x4A8A`。
- `RH_IN = 相对湿度 %RH × 512`，16 位，先 LSB 后 MSB；50 %RH 示例为 `0x6400`。

依据：[ENS160 Datasheet §16.2.5、§16.2.6](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)。DFRobot 库的 `setTempAndHum()` 使用相同换算，并从 `0x13` 连续写 4 字节：[DFRobot_ENS160.cpp](https://github.com/DFRobot/DFRobot_ENS160/blob/main/DFRobot_ENS160.cpp)。DFRobot 示例中的 25 °C/50 %RH 只是演示值，产品代码应来自真实外部传感器；没有外部传感器时必须把固定值及由此产生的不确定性写入设备元数据。

## ESP-IDF 6.0.1 集成判断

### DFRobot Arduino API 不能直接用于当前工程

[DFRobot Arduino 库](https://github.com/DFRobot/DFRobot_ENS160) 当前 `library.properties` 版本为 1.0.1，源码使用 Arduino 的 `TwoWire`、`SPIClass`、`Wire.begin()`、`delay()`、`pinMode()` 与 `digitalWrite()`。其 `architectures=*` 表示 Arduino Library Manager 的架构声明，不代表它可以在不含 Arduino Core 的纯 ESP-IDF 工程中编译。本工程 `platformio.ini` 明确为 `framework = espidf`，所以不能直接使用：

```cpp
#include <DFRobot_ENS160.h>
DFRobot_ENS160_I2C ENS160(&Wire, 0x53);
```

可行方案按优先级为：

1. 为本工程编写原生 ESP-IDF 组件，只移植寄存器协议和换算公式；这是当前架构下最小、可测试的方案。
2. 若必须复用 DFRobot C++ API，先正式评估并引入 Arduino as an ESP-IDF component，同时处理 C++ 构建、默认引脚、组件生命周期和代码体积；这属于工程架构变更，不是添加一个 `lib_deps` 即可完成。
3. 不建议把整个工程切换为 Arduino framework，只为接入一个寄存器型 I²C 传感器。

DFRobot 库 `begin()` 会调用无参数的 `Wire.begin()`，并由库自行切到 Standard 模式；这会绕过本工程共享 I²C 总线的统一初始化。即便引入 Arduino component，也应核对它是否会重复占用总线或选择错误默认 GPIO。[DFRobot_ENS160.cpp](https://github.com/DFRobot/DFRobot_ENS160/blob/main/DFRobot_ENS160.cpp)

### 原生 ESP-IDF 落地要点

- 使用 ESP-IDF 6.0.1 的新主机驱动 `driver/i2c_master.h`；组件在 CMake 中声明 `PRIV_REQUIRES esp_driver_i2c`。
- XIAO 三板统一用工程符号 `XIAO_I2C_SDA`（D4）和 `XIAO_I2C_SCL`（D5），不要把某一板的裸 GPIO 写死。当前映射：C3 为 GPIO6/7，S3 为 GPIO5/6，C6 为 GPIO22/23。
- 设备配置使用原始 7 位地址 `0x53`（默认模块）或 `0x52`，不要左移或附加读写位。
- 先用 100 kHz；虽然 ENS160 芯片支持到 1 MHz，但 ESP32-S3 的 ESP-IDF 文档规定主机 SCL 不应超过 400 kHz，而且模块与接线并未在本项目内验证 1 MHz。
- 用 `i2c_master_probe()` 诊断地址，用 `i2c_master_transmit_receive()` 完成“写一个寄存器地址、重复起始、读数据”的寄存器访问。设置有限超时并处理 NACK/timeout。
- 初始化时读取 `PART_ID=0x0160`，再进入 Standard；周期采样时先判断错误位、`VALIDITY` 和 `NEWDAT`，然后连续读取输出。
- 总线由单一组件持有；多个 FreeRTOS 任务不要各自创建同一设备或重复初始化 SDA/SCL。释放顺序是先移除设备，再删除总线。

ESP-IDF 依据：[ESP-IDF 6.0.1 ESP32-S3 I²C Programming Guide](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html)。芯片 I²C 能力依据：[ENS160 Datasheet §14.1](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)。

## 来源冲突与不确定项

| 项目 | 发现 | 采用规则 |
|---|---|---|
| 模块与裸芯片供电 | DFRobot 模块写 3.3 V；ENS160 裸芯片 VDD 仅 1.71–1.98 V | 各自保留并明确层级，不相互替代；不推断模块支持 5 V |
| 默认 I²C 地址 | SEN0515 Wiki 和示例明确默认 `0x53`；通用 Arduino 类构造函数默认参数却是 `0x52` | 对 SEN0515 显式传 `0x53`；启动时探测并核对实物 SDO 接法 |
| “预热小于 3 分钟” | DFRobot 产品简介简化为小于 3 分钟；ScioSense 定义典型 3 分钟 warm-up，另有首次 1 小时启动等阶段 | 使用 ScioSense Production 数据表的分阶段表述，不声称 3 分钟后达到最终准确度 |
| 低功耗测量 | ScioSense ENS16x 家族页写低至 150 µA；ENS160 数据表只有不测气体的 Deep Sleep/Idle 和 29 mA Standard | 不把 ENS161 的 `0x03`/`0x04` 或家族 150 µA 宣称移植到 SEN0515 |
| 精度 | DFRobot 使用“高准确性”等定性描述；原厂仅给有限测试条件下的氢示例误差 | 不生成 TVOC/eCO2 全局精度数字，保留测试条件 |
| 应用说明元数据 | 2025 通信 AN v0.7 封面日期为 2025-07-31，修订表日期为 2025-07-18，并标为 Preliminary；示例还混用了 `ENS16x_I2C_ADDRESS`/`ENS161_I2C_ADDRESS` | 记录两个日期；冲突时以 ENS160 v1.3 Production 数据表为准，不照抄示例符号 |
| 模块上拉与接口选择电路 | DFRobot 页面未提供可直接核对的完整模块原理图；PCB Package 下载是封装/符号资源，不足以证明上拉阻值和 CS 默认电路 | 上板前用万用表/实物照片或后续官方原理图核对；文档不虚构阻值 |

## 一手来源清单

所有来源均在 2026-07-19 检索：

1. [DFRobot SEN0515 中文 Wiki](https://wiki.dfrobot.com.cn/_SKU_SEN0515_Fermion_ENS160_Air_Quality_Sensor_)：模块规格、引脚、默认地址、Arduino 示例和状态说明。
2. [DFRobot SEN0515 英文 Reference](https://wiki.dfrobot.com/sen0515/docs/20714)：当前英文 API、协议与原理入口；页面标注最后修订 2025-12-17。
3. [DFRobot/DFRobot_ENS160](https://github.com/DFRobot/DFRobot_ENS160)：MIT Arduino/Python 库；`library.properties` 为 1.0.1，检索时 main 最新代码提交为 `7a45a70`（2022-08-16）。
4. [ScioSense ENS160 Datasheet v1.3](https://www.sciosense.com/wp-content/uploads/2023/12/ENS160-Datasheet.pdf)：Production，文件内发布日期 2023-03-29，文档号 SC-001224-DS-9。URL 中的 `2023/12` 不是文件内发布日期。
5. [ScioSense ENS16x Fundamental Communication Application Note v0.7](https://www.sciosense.com/wp-content/uploads/2025/08/ENS16x-Application-Note-Fundamental-Communication.pdf)：封面日期 2025-07-31、修订表日期 2025-07-18，Preliminary；仅用于补充通信示例与 ENS161-only 模式边界。
6. [ScioSense ENS16x 产品页](https://www.sciosense.com/ens16x-digital-metal-oxide-multi-gas-sensor-family/)：当前原厂下载入口和家族级能力；家族描述不能覆盖 ENS160 型号数据表。
7. [ESP-IDF 6.0.1 ESP32-S3 I²C 文档](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-reference/peripherals/i2c.html)：当前工程基线的 I²C master API、速率、上拉和地址传参规则。
