# XIAO ESP32-S3 OLED 编码器控制台验收固件

当前活动构建用于验证以下实物组合：

- Seeed Studio XIAO ESP32-S3
- 外接 1.3 寸 SH1106 128×64 OLED 编码器控制板
- 板载旋转编码器、PUSH、BACK 与 CONFIRM 按键

当前固件在编码器和屏幕动画基础上，增加了四线 12V 风扇的物理 PWM 与转速反馈。D3/GPIO4 使用 ESP-IDF LEDC 输出 25kHz，并通过外接 AOD4184L N-MOS 开漏级驱动风扇 PWM 线；D4/GPIO5 使用 ESP-IDF PCNT 读取 TACH。原有 C6 风扇电源开关、Matter、NVS 与堵转保护源码仍保留，但不会被当前 `main/CMakeLists.txt` 编译。

## 交互行为

- 顺/逆时针旋转：每次由官方编码器组件识别的旋转事件固定改变 `1%`，范围 `0–100%`，无加速度。
- 编码器 PUSH 单击：由官方按键组件在 300ms 双击判定窗口后进入 `RUN`，D3 输出当前目标占空比。
- 编码器 PUSH 双击：由官方按键组件进入 `STOP`，持续拉低风扇 PWM 控制线；部分风扇可能只降到最低转速。
- BACK / CONFIRM：分别切换到上一页 / 下一页，三页循环浏览。
- OLED：主控页显示 `ON/OFF`、动画 PWM 百分比、线性进度条与每秒更新的真实 RPM。

屏幕现包含三页紧凑信息：

1. `PWM` 主控页：运行状态、真实 RPM、动画占空比与进度条；
2. `INPUT 2/3` 输入诊断页：目标值、动画显示值、官方 knob 计数及最后旋转方向；
3. `SYSTEM 3/3` 系统页：运行时间、剩余堆内存、OLED 型号、电压、I²C 地址和固件版本。

切页动画使用 LVGL 自带 `lv_anim_path_ease_out`，仅滑动 20px、持续 120ms；占空比动画继续使用 `lv_anim_path_linear`。

## 风扇 PWM 与测速接线

AOD4184L 使用开漏方式拉低风扇 PWM 控制线：

```text
D3/GPIO4 ── 470R ── Gate
                       │
                      10k
                       │
GND ───────────────── Source

Drain ── 风扇 PWM 控制线

3V3 ── 10k ──┬── D4/GPIO5
              └── 风扇 TACH 线
```

XIAO GND、风扇 GND 与 12V 电源负极必须共地。风扇 12V 由外部电源直接提供，不能接入 XIAO。TACH 只允许用 10k 上拉到 3.3V；ESP32-S3 GPIO 不能接 5V。固件启动时会尽早把 PWM 置为 0%，但如果目标风扇在 0% 下仍保持最低转速，必须增加独立的 12V 电源开关才能物理停转。

百分比和进度条使用 LVGL 自带 `lv_anim_path_linear` 线性补间；大幅变化按恒定 `100%/s` 追随，小幅变化至少播放 70ms，避免逐格跳变。

## 使用的现成组件

交互与绘制不再维护自写状态机或私有帧缓冲渲染器，全部从 Espressif Component Registry 固定版本解析：

- `espressif/knob 1.1.0`：EC10 旋转识别和消抖，当前以 2ms 周期、1 个采样周期消抖；
- `espressif/button 4.2.0`：单击、双击和按键消抖；
- `espressif/esp_lvgl_port 2.9.0`：ESP-IDF LCD 与 LVGL 的线程安全适配；
- `lvgl/lvgl 9.5.0`：标签、进度条和线性动画。
- `tny-robotics/sh1106-esp-idf 1.0.1`：通过 ESP-IDF 标准 LCD 接口驱动 SH1106。

版本与校验哈希记录在 `main/idf_component.yml` 和 `dependencies.lock` 中。

## 安全接线

| 控制板信号 | XIAO ESP32-S3 | 芯片 GPIO |
|---|---|---:|
| `VCC` | `3V3` | — |
| `GND` | `GND` | — |
| `PUSH` | `D0` | GPIO1 |
| `ENCODER_A` | `D1` | GPIO2 |
| `ENCODER_B` | `D2` | GPIO3 |
| `FAN_PWM` | `D3` | GPIO4 |
| `FAN_TACH` | `D4` | GPIO5 |
| `CONFIRM` | `D7` | GPIO44 |
| `BACK` | `D8` | GPIO7 |
| `SCL` | `D9` | GPIO8 |
| `SDA` | `D10` | GPIO9 |

XIAO 可以继续通过 USB/5V 供电，但外接控制板的 `VCC` 必须接 `3V3`，不能接 5V。ESP32-S3 GPIO 不耐 5V。OLED I²C 地址保持 `0x3C`。

## 构建与测试

```bash
python3 scripts/validate_project.py .
python3 -m unittest discover -s tests -v
pio run -e xiao_esp32s3
```

活动工具链固定为 `platformio/espressif32@7.0.1`，集成 ESP-IDF `6.0.1`。XIAO S3 配置为 8MB Flash 和 Octal PSRAM。

## 真机边界

刷写只应在确认当前串口属于这块 XIAO ESP32-S3 后执行。启动成功标记为：

```text
EC10_FAN_PWM_READY version=12 board=seeed_xiao_esp32s3_sh1106_encoder_console
```

真机验收步骤见 [EC10、OLED、PWM 与 RPM 验收说明](docs/ec10-display-demo.md)。当前仍不初始化独立的风扇电源切换。

## 一手资料

- [Seeed XIAO ESP32-S3 引脚与硬件说明](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- [SH1106 ESP-IDF 组件](https://components.espressif.com/components/tny-robotics/sh1106-esp-idf)
- [Espressif knob 组件](https://components.espressif.com/components/espressif/knob)
- [Espressif button 组件](https://components.espressif.com/components/espressif/button)
- [Espressif esp_lvgl_port 组件](https://components.espressif.com/components/espressif/esp_lvgl_port)
- [ESP-IDF ESP32-S3 PCNT 驱动](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/pcnt.html)

资料核对日期：2026-08-16。
