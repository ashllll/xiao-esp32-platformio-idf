# 编码器控制台与 OLED 初步验收

## 当前硬件

当前组合改为 XIAO ESP32-S3 和外接的 1.3 寸 OLED 编码器控制板。控制板集成 SH1106 128×64 OLED、旋转编码器按压、BACK 和 CONFIRM 两枚按键；I²C 地址为 0x3C。

## 接线复核

1. XIAO 可经 USB 供电，但控制板 `VCC` 接 XIAO `3V3`，不能接 5V；两块板共地。
2. `SDA` 接 D10/GPIO9，`SCL` 接 D9/GPIO8。
3. `BACK` 接 D8/GPIO7，`CONFIRM` 接 D7/GPIO44。
4. 编码器 `PUSH` 接 D0/GPIO1，`ENCODER_A` 接 D1/GPIO2，`ENCODER_B` 接 D2/GPIO3。
5. D0–D10 是 XIAO 板载丝印名，固件内部使用表中的芯片 GPIO 编号。
6. 风扇转速反馈 `TACH` 接 D4/GPIO5，并使用 10kΩ 电阻上拉到 XIAO `3V3`；不得上拉到 5V 或 12V。风扇 12V 负极与 XIAO GND 必须共地。

## 固件行为

旋转输入由 Espressif `knob 1.1.0` 负责识别；A/B 使用 GPIO2/GPIO3，不使用工程私有的 Gray-code 状态机。每次组件上报 `KNOB_LEFT` 或 `KNOB_RIGHT`，目标值线性改变 1%，并限制在 0–100%。

按键由 Espressif `button 4.2.0` 负责消抖和点击分类。编码器 PUSH 的单击需要等待约 300ms 确认没有第二击后才进入 RUN；双击只进入 STOP，不会重复发出单击事件。BACK 与 CONFIRM 已由同一组件接入，分别循环切换上一页和下一页。

屏幕由 `esp_lvgl_port 2.9.0` 和 `LVGL 9.5.0` 驱动，使用 LVGL 自带控件和动画：

- SH1106 使用 Component Registry 中的 `tny-robotics/sh1106-esp-idf 1.0.1` 标准 `esp_lcd_panel` 驱动；
- 风扇 PWM 使用 ESP-IDF 官方 LEDC 驱动，在 D3/GPIO4 输出 25kHz，并通过 AOD4184L N-MOS 开漏级连接风扇控制线；
- `esp_lvgl_port` 的 I1 页格式转换会交换位极性，而 SH1106 组件不提供硬件反相；固件因此在 LVGL 主题层交换黑白，物理屏仍显示黑底白字；
- 显示配置保留 SH1106 驱动的默认 COM 扫描方向，不继承旧扩展板的 180 度翻转；
- PWM 百分比和进度条使用 `lv_anim_path_linear` 接近旋钮目标；
- 大幅变化按恒定 100%/s 播放，小幅变化保留至少 70ms 动画；
- 顶部使用 8px 点阵字体显示 `PWM` 状态和真实 `RPM`，避免灰阶字体在 1-bit 屏上发虚；
- 中央占空比使用 LVGL 内置的原生 16px UNSCII 1 bpp 字体；不使用对象缩放，避免绘制尺寸与布局盒不一致造成错位；
- `ON/OFF` 状态块使用黑白反相反馈，不使用持续旋转装饰；
- D4/GPIO5 的测速输入由 ESP-IDF 官方 PCNT 驱动计数，启用 1µs 毛刺滤波；默认按每转 2 脉冲、1 秒采样窗换算 RPM，显示分辨率为 30 RPM。

界面分为三页：

- `PWM`：主操作页，显示 RUN/STOP、RPM 占位、占空比和线性进度条；
- `INPUT 2/3`：显示 `TARGET`、`DISPLAY`、官方 knob 累计计数和最后一次 `CW/CCW` 方向；
- `SYSTEM 3/3`：显示设备运行秒数、剩余堆内存、`SH1106 3V3` 以及 `PWM3 TACH4 F12`。

BACK/CONFIRM 切页采用 LVGL 内置 `lv_anim_path_ease_out`，20px 短距离滑入、120ms 完成。动画期间不改变旋钮目标值，也不改变 PUSH 单/双击语义。

## 真机验收清单

1. 上电后屏幕显示 PWM/OFF、0%、RPM 0，无重启循环。
2. 顺时针逐卡点旋转，目标按 1% 增加；反向旋转按 1% 减少；边界固定在 0% 和 100%。
3. 快速旋转后，百分比和进度条沿直线速度追上目标，不回弹。
4. 按下编码器单击后约 300ms 状态块反相显示 ON，风扇按目标占空比运行；双击后立即恢复 OFF，并把 PWM 控制线保持为低电平。
5. 状态切换不影响占空比设定值，主数字和底部进度条保持同步。
6. 按 CONFIRM 依次进入 INPUT、SYSTEM、PWM；按 BACK 反向循环，切页无错位或残影。
7. INPUT 页旋转时 `TARGET` 先变化，`DISPLAY` 平滑追随，`ENC` 与 `LAST CW/CCW` 同步更新。
8. SYSTEM 页运行时间递增、堆内存数值稳定，OLED/PWM/固件信息不越界。
9. 连续操作 2 分钟，确认无误触发、漏步、屏幕花屏或看门狗复位。
10. 复位时风扇先全速保护；固件启动完成后进入 OFF。单击启动后旋钮从 0% 到 100% 应线性改变实际风扇控制占空比。
11. 风扇旋转后右上角 RPM 每秒更新；固定占空比下数值应大致稳定，提高占空比后 RPM 应整体上升。若风扇仍在低速旋转，即使界面为 OFF，RPM 也会如实显示非零。

串口同时应输出 `EC10 duty=...`、`EC10 single click: RUN`、`EC10 double click: STOP`、`BACK/CONFIRM page=...`，以及递增的 `HEARTBEAT sequence=... rpm=... tach_pulses=... page=... heap_kb=...`。测速由 PCNT 硬件完成，不运行临时 GPIO 轮询计数任务。

## 未覆盖内容

本阶段初始化四线风扇的 PWM 和 TACH，但不初始化独立的风扇电源切换、Matter 或 NVS。12V 仍由外部电源直接供给，因此 PWM=0% 是否真正停转仍由风扇本身决定。构建成功只表示固件路径通过，真实 RPM 与停转行为仍需上板验收。
