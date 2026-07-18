# 引脚定义

下表是 XIAO 顶部 D0–D10 排针映射；代码定义位于 `include/xiao_pins.h`。

| XIAO | ESP32C3 | ESP32S3 | ESP32C6 | 常用功能 |
|---|---:|---:|---:|---|
| D0 | GPIO2 | GPIO1 | GPIO0 | ADC/GPIO |
| D1 | GPIO3 | GPIO2 | GPIO1 | ADC/GPIO |
| D2 | GPIO4 | GPIO3 | GPIO2 | ADC/GPIO |
| D3 | GPIO5 | GPIO4 | GPIO21 | GPIO |
| D4 | GPIO6 | GPIO5 | GPIO22 | I²C SDA |
| D5 | GPIO7 | GPIO6 | GPIO23 | I²C SCL |
| D6 | GPIO21 | GPIO43 | GPIO16 | UART TX |
| D7 | GPIO20 | GPIO44 | GPIO17 | UART RX |
| D8 | GPIO8 | GPIO7 | GPIO19 | SPI SCK |
| D9 | GPIO9 | GPIO8 | GPIO20 | SPI MISO |
| D10 | GPIO10 | GPIO9 | GPIO18 | SPI MOSI |

## 使用提醒

- ESP32C3 的 GPIO2、GPIO8、GPIO9 是启动配置相关引脚；外围电路不要在复位时强制错误电平。
- ESP32C3 的 D6 会输出启动日志，作为输入使用时要特别谨慎。
- ESP32S3 的 GPIO19/20 通常用于原生 USB，不应随意改作普通 GPIO。
- S3 Sense 的 microSD 会占用 GPIO7、GPIO8、GPIO9、GPIO21。
- 接入 5 V 外设前确认其逻辑电平；ESP32 GPIO 不耐受 5 V。

