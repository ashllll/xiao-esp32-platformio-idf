# XIAO ESP32-S3 PWM fan controller

This example is a source snapshot of the current XIAO ESP32-S3 fan-controller
prototype:

- Seeed Studio XIAO ESP32-S3;
- SH1106 128×64 OLED encoder console;
- EC10 encoder with PUSH, BACK, and CONFIRM inputs;
- four-wire 12 V PWM fan;
- D3/GPIO4 PWM output and D4/GPIO5 tachometer feedback.

The source lives in
[`examples/xiao_esp32s3_pwm_fan_controller`](https://github.com/ashllll/xiao-esp32-platformio-idf/tree/main/examples/xiao_esp32s3_pwm_fan_controller).

## Current behavior

- Encoder rotation changes the 0–100% target in 1% steps.
- A PUSH single click enters RUN; a double click enters STOP.
- The OLED shows state, PWM, linear motion, and measured RPM.
- ESP-IDF LEDC generates 25 kHz PWM on D3.
- ESP-IDF PCNT samples TACH pulses on D4 once per second and defaults to two
  pulses per revolution.
- Serial heartbeats report target, displayed value, RPM, and raw pulse count.

## Electrical boundary

TACH is commonly an open-collector output. This example requires a 10 kΩ
pull-up from D4 to the XIAO 3.3 V rail and a common ground with the fan's 12 V
supply. D4 is not 5 V tolerant; never pull TACH up to 5 V or 12 V.

The current prototype has no independent 12 V power switch. Some four-wire fans
continue at their minimum speed with PWM=0%. A default-off high-side supply
switch is required when physical stop at power-up is mandatory.

## Build and validation

```bash
python3 examples/xiao_esp32s3_pwm_fan_controller/scripts/validate_project.py \
  examples/xiao_esp32s3_pwm_fan_controller
python3 -m unittest discover \
  -s examples/xiao_esp32s3_pwm_fan_controller/tests -v
pio run -d examples/xiao_esp32s3_pwm_fan_controller -e xiao_esp32s3
```

This snapshot passed the XIAO ESP32-S3 firmware build and all 11 host tests.
Measured RPM still requires target-fan validation of pulse count, signal level,
and actual speed.
