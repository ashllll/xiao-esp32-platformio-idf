# XIAO ESP32-S3 PWM 风扇控制器

这个示例是当前 XIAO ESP32-S3 风扇控制器原型的源码快照，组合如下：

- Seeed Studio XIAO ESP32-S3；
- SH1106 128×64 OLED 编码器控制板；
- EC10 旋转编码器、PUSH、BACK、CONFIRM；
- 四线 12V PWM 风扇；
- D3/GPIO4 PWM 输出和 D4/GPIO5 TACH 转速反馈。

源码位于
[`examples/xiao_esp32s3_pwm_fan_controller`](https://github.com/ashllll/xiao-esp32-platformio-idf/tree/main/examples/xiao_esp32s3_pwm_fan_controller)。

## 当前功能

- 旋转编码器以 1% 步进调整 0–100% 目标占空比；
- PUSH 单击进入 RUN，双击进入 STOP；
- OLED 显示运行状态、PWM、线性动画和真实 RPM；
- ESP-IDF LEDC 在 D3 输出 25kHz PWM；
- ESP-IDF PCNT 在 D4 每秒统计 TACH 脉冲，默认按每转 2 脉冲换算；
- 串口心跳同时报告目标值、显示值、RPM 和脉冲数。

## 电气边界

TACH 通常是开集电极输出。本示例要求 D4 使用 10kΩ 上拉到 XIAO
3.3V，并与风扇 12V 电源负极共地。D4 不耐 5V，禁止把 TACH
上拉到 5V 或 12V。

当前原型没有独立的 12V 电源开关。部分四线风扇在 PWM=0% 时仍保持最低
转速；如果必须做到上电静止，需要另加默认关闭的高边电源开关，不能仅靠
PWM 控制线保证。

## 构建与验证

```bash
python3 examples/xiao_esp32s3_pwm_fan_controller/scripts/validate_project.py \
  examples/xiao_esp32s3_pwm_fan_controller
python3 -m unittest discover \
  -s examples/xiao_esp32s3_pwm_fan_controller/tests -v
pio run -d examples/xiao_esp32s3_pwm_fan_controller -e xiao_esp32s3
```

本次快照在 XIAO ESP32-S3 工具链构建通过，主机测试 11 项通过；真实 RPM
是否正确仍需在具体风扇上核对脉冲数、接线电平和实测转速。
