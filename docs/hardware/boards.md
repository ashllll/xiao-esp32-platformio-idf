# XIAO 型号

| PlatformIO 环境 | Board ID | 芯片 | 默认 Flash |
|---|---|---|---:|
| `xiao_esp32c3` | `seeed_xiao_esp32c3` | ESP32-C3 | 4 MB |
| `xiao_esp32s3` | `seeed_xiao_esp32s3` | ESP32-S3 | 8 MB |
| `xiao_esp32c6` | `seeed_xiao_esp32c6` | ESP32-C6 | 4 MB |

构建平台固定为 `platformio/espressif32@7.0.0`，对应 ESP-IDF 6.0.0 工具链。宿主环境为 Apple Silicon `darwin_arm64`。

官方资料：

- [XIAO ESP32C3 入门](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)
- [XIAO ESP32S3 入门](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)
- [XIAO ESP32C6 入门](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/)

!!! note
    XIAO ESP32S3 Sense、Plus 等衍生型号会占用额外 GPIO。使用摄像头、SD 卡或扩展底板前，请在外围设备文档中记录冲突。
