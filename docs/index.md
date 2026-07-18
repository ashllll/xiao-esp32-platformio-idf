# XIAO ESP32 项目

这是一个可直接复用的 PlatformIO + ESP-IDF 工程和开发文档模板，支持：

- Seeed Studio XIAO ESP32C3
- Seeed Studio XIAO ESP32S3
- Seeed Studio XIAO ESP32C6

代码中的板级引脚统一由 `include/xiao_pins.h` 提供。新增项目时，优先扩展外围设备组件和对应文档，不要在业务代码中散布裸 GPIO 数字。

```mermaid
flowchart LR
    A[需求与接线] --> B[外围设备组件]
    B --> C[PlatformIO 构建]
    C --> D[烧录与验证]
    D --> E[固件与文档发布]
```

