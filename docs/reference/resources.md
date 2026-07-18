# 资料索引

本工程使用两层资料：工程内保存经过复核的精简说明；完整官方归档、PDF、原理图、模型和历史资料保留在独立资料库中。

## 本地资料库

资料库路径：

```text
/Users/llll/ESP32_资料整理
```

重点入口：

| 内容 | 本地路径 | 用途 |
|---|---|---|
| 总索引 | `README.md`、`inventory_by_category.md` | 查找已有代码、文档和硬件资料 |
| 结构化清单 | `manifest.csv` | 按路径、类型、时间和来源检索 |
| S3 官方归档 | `官方文档/Seeed_XIAO_ESP32S3_Getting_Started/` | 官方中文原文、全系列镜像和资源清单 |
| 历史 PlatformIO 项目 | `02_PlatformIO项目/` | 查找已经验证过的接线与实现 |
| 硬件资料 | `04_硬件_数据手册/` | ADS1115 等外围设备数据手册和原理图 |
| 原 XIAO 工作目录 | `归档/SeeedStudioXIAOESP32_2026-07-18/` | 原始文档、组件、压缩包和工作环境完整归档 |

!!! warning
    `03_文档资料/` 中包含历史派生文档。若引脚、摄像头、SD 卡、功耗或板卡版本信息冲突，以 Seeed 官方文档和本工程当前引脚表为准。

## Seeed 官方入口

| 型号 | 官方中文文档 | 官方 GitHub Markdown |
|---|---|---|
| XIAO ESP32C3 | [快速上手](https://wiki.seeedstudio.com/cn/XIAO_ESP32C3_Getting_Started/) | [原始文档](https://github.com/Seeed-Studio/wiki-documents/blob/docusaurus-version/sites/zh-CN/docs/Sensor/SeeedStudio_XIAO/SeeedStudio_XIAO_ESP32C3/cn_XIAO_ESP32C3_Getting_Started.md) |
| XIAO ESP32S3 | [快速上手](https://wiki.seeedstudio.com/cn/xiao_esp32s3_getting_started/) | [原始文档](https://github.com/Seeed-Studio/wiki-documents/blob/docusaurus-version/sites/zh-CN/docs/Sensor/SeeedStudio_XIAO/SeeedStudio_XIAO_ESP32S3/cn_XIAO_ESP32S3_Getting_Started.md) |
| XIAO ESP32C6 | [快速上手](https://wiki.seeedstudio.com/cn/xiao_esp32c6_getting_started/) | [原始文档](https://github.com/Seeed-Studio/wiki-documents/blob/docusaurus-version/sites/zh-CN/docs/Sensor/SeeedStudio_XIAO/SeeedStudio_XIAO_ESP32C6/cn_XIAO_ESP32C6_Getting_Started.md) |

还应结合以下权威来源：

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [PlatformIO Espressif 32 平台](https://registry.platformio.org/platforms/platformio/espressif32)
- [PlatformIO XIAO ESP32C3](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32c3.html)
- [PlatformIO XIAO ESP32S3](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32s3.html)
- [PlatformIO XIAO ESP32C6](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32c6.html)

## 已复核的重要差异

- ESP32C3 的 GPIO2、GPIO8、GPIO9 涉及启动配置；D6 会输出启动日志。
- ESP32S3 基础版、Sense 与 Plus 的扩展引脚并不完全相同，不能共用一张扩展引脚表。
- S3 Sense 的 microSD 会占用 GPIO7、GPIO8、GPIO9、GPIO21，并与排针 SPI/用户 LED 资源产生冲突。
- S3 Sense 后续批次的摄像头型号可能不同，应用应以当前硬件和官方说明为准，避免只写死旧型号。
- ESP32C6 具备 802.15.4 能力，但具体 Zigbee/Thread 功能仍取决于所用 ESP-IDF、组件和项目配置。

本页最后核对日期：2026-07-18。

当前工程工具链基线：PlatformIO Core 6.1.19、Espressif 32 Platform 7.0.0、ESP-IDF 6.0.0、Apple Silicon `darwin_arm64`。
