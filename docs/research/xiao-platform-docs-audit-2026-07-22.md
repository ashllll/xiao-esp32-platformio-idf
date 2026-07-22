# XIAO / PlatformIO / ESP-IDF 文档声明核对

检索日期：2026-07-22
Review date: 2026-07-22

本文审查仓库中关于 Seeed Studio XIAO ESP32C3、ESP32S3、ESP32C6、PlatformIO、ESP-IDF 和 MkDocs Material 文档站的核心技术声明。结论只使用 Seeed Studio、Espressif、PlatformIO、MkDocs 和 Material for MkDocs 的一手官方资料；仓库自身文件只用于定位待核对声明，不作为外部事实的证明。

This note reviews the repository's core technical claims about the Seeed Studio XIAO ESP32C3, ESP32S3, ESP32C6, PlatformIO, ESP-IDF, and the MkDocs Material documentation site. Conclusions use first-party sources only. Repository files identify claims under review but do not independently prove external hardware or toolchain facts.

## 结论摘要 / Executive summary

- **正确 / Correct**：三块板的 D0–D10 GPIO 映射、PlatformIO Board ID、Flash/PSRAM 主表，以及 `espressif32@7.0.1` 对应 ESP-IDF 6.0.1 的版本关系均有官方依据。
- **错误 / Incorrect**：`docs/hardware/boards.md` 把 ESP32-C6 简写为“RISC-V，单核”。XIAO C6 的官方资料明确列出一个最高 160 MHz 的高性能 RISC-V 处理器和一个最高 20 MHz 的低功耗 RISC-V 处理器。建议改成“32-bit RISC-V；单核 HP CPU + LP RISC-V core”，避免与 C3 的单核描述混同。
- **错误 / Incorrect**：`docs/hardware/boards.md`、`docs/hardware/pinout.md` 和 `docs/reference/resources.md` 声称 S3 Sense microSD 占用 GPIO7、GPIO8、GPIO9、GPIO21。Seeed 当前引脚表给出 `CS=GPIO3`、`SCK=GPIO7`、`MISO=GPIO8`、`MOSI=GPIO9`；GPIO21 是用户 LED。应将 GPIO21 改为 GPIO3，并说明它分别冲突 D2、D8、D9、D10，而不是用户 LED。
- **需修正表述 / Needs clarification**：当前 S3 Sense 相机并非单一固定型号。Seeed 明确记录早期 OV2640 已停产、后续板使用 OV3660，并提供兼容 OV5640 的替换方案。仓库“批次可能变化、应核对实物”的边界是正确的，但双语页面应明确列出 OV2640 / OV3660 / 可替换 OV5640。
- **需修正站点结构 / Site structure needs correction**：Material 的 `theme.language: zh` 只设置一个 MkDocs 项目的规范语言和界面翻译，不会翻译页面内容，也不能单独形成双语站。Material 官方建议每种语言建立一个子目录项目，再用 `extra.alternate` 语言选择器互链。若采用第三方 i18n 插件，它属于额外实现选择，不能把 `theme.language` 本身描述为双语能力。
- **证据边界 / Evidence boundary**：官方板卡资料和 PlatformIO manifest 能证明设计规格与构建目标；不能证明某块实物已运行当前提交。仓库现有“构建通过”和“真机通过”分层应保留。

## 逐项核对 / Claim-by-claim review

| 状态 | 仓库声明 / Repository claim | 核对结论与建议 / Finding and correction | 官方依据 / First-party source |
|---|---|---|---|
| 正确 | `platform = espressif32@7.0.1` 对应 ESP-IDF 6.0.1 | PlatformIO 7.0.1 release notes 明确列出 ESP-IDF 6.0.1；版本固定关系正确。 | [PlatformIO Espressif32 7.0.1 release](https://github.com/platformio/platform-espressif32/releases/tag/v7.0.1)；[PlatformIO Registry: espressif32](https://registry.platformio.org/platforms/platformio/espressif32) |
| 正确 | 三个 Board ID 分别为 `seeed_xiao_esp32c3`、`seeed_xiao_esp32s3`、`seeed_xiao_esp32c6` | 与 PlatformIO 官方 board 页面一致。 | [C3](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32c3.html)、[S3](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32s3.html)、[C6](https://docs.platformio.org/en/latest/boards/espressif32/seeed_xiao_esp32c6.html) |
| 正确 | C3 / C6 为 4 MB Flash；S3 基础版为 8 MB Flash + 8 MB Octal PSRAM | 与 Seeed 当前规格一致。S3 Plus 为 16 MB Flash，不能继承基础版环境而仍声称完整匹配。 | [XIAO C3 getting started](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)、[XIAO S3 series getting started](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)、[XIAO C6 getting started](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/) |
| 正确 | S3 的 PSRAM 应按 Octal 模式配置 | Seeed 的 ESP-IDF/FreeRTOS 指南特别指出 XIAO S3 使用 Octal PSRAM，不是 Quad。 | [XIAO ESP32S3 with FreeRTOS](https://wiki.seeedstudio.com/xiao-esp32s3-freertos/) |
| 正确 | D0–D10 GPIO 主表 | 仓库 `docs/hardware/pinout.md` 的 C3/S3/C6 数字与 Seeed 当前 Pin Map 一致。 | [C3 Pin Map](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#pin-map)、[S3 Pin Map](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#hardware-overview)、[C6 Pin Map](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#hardware-overview) |
| 错误 | C6 是“RISC-V，单核” | 该写法遗漏 LP RISC-V core。建议写“32-bit RISC-V；单核 HP CPU（最高 160 MHz）+ LP RISC-V core（最高 20 MHz）”。若表格只描述应用 CPU，应把列名改成“主应用 CPU”。 | [Seeed XIAO ESP32C6 overview](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/)、[Espressif ESP32-C6 datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf) |
| 正确 | C3 为 32-bit RISC-V 单核，最高 160 MHz | 与 Seeed C3 规格一致。 | [Seeed XIAO ESP32C3 specifications](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#specifications) |
| 正确 | S3 为 Xtensa LX7 双核，最高 240 MHz | 与 Seeed S3 系列规格一致。 | [Seeed XIAO ESP32S3 specifications](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#specification) |
| 错误 | S3 Sense microSD 占用 GPIO7、8、9、21 | 当前官方表为 `CS GPIO3`、`SCK GPIO7`、`MISO GPIO8`、`MOSI GPIO9`。GPIO21 是用户 LED，不是 SD CS。应同步修正所有出现位置。 | [Seeed XIAO ESP32S3 Sense Pin Map](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#hardware-overview) |
| 正确但需展开 | S3 Sense 摄像头型号可能随批次变化 | 官方明确区分已停产 OV2640、后续 OV3660，以及兼容替换的 OV5640。建议在中英页面直接写出型号和边界。 | [Seeed XIAO ESP32S3 camera notice](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#introduction) |
| 正确 | S3 用户 LED 为 GPIO21、低电平点亮 | GPIO 和 active-low 行为均有 Seeed 官方示例支持。 | [XIAO ESP32S3 with FreeRTOS](https://wiki.seeedstudio.com/xiao-esp32s3-freertos/) |
| 正确 | C3 没有可由应用控制的板载用户 LED | Seeed 的 Blink 指南明确要求外接 D10 LED，并写明没有 `LED_BUILTIN`；板上 Light 是充电 LED。 | [XIAO ESP32C3 getting started](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/) |
| 正确 | C6 用户灯位于 GPIO15 | Seeed C6 Pin Map 将 `Light` 映射为 GPIO15。 | [XIAO ESP32C6 Pin Map](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#pin-map) |
| 正确 | C3 的 GPIO2、GPIO8、GPIO9 是 strapping pins | Seeed 页面明确提醒这些引脚会影响启动模式。 | [XIAO ESP32C3 strapping pins](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#strapping-pins) |
| 正确 | S3 的 GPIO19 / GPIO20 用于 USB D- / D+，依赖原生 USB 时不应改作普通 GPIO | Espressif 的 USB Serial/JTAG 文档明确给出 GPIO19=D-、GPIO20=D+。 | [ESP-IDF USB Serial/JTAG Console](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32s3/api-guides/usb-serial-jtag-console.html) |
| 正确 | ESP-IDF 6.0 新组件应使用 `driver/i2c_master.h`；旧 `driver/i2c.h` 已标 EOL | Espressif v6.0 migration guide 明确标记 legacy I²C driver 为 EOL，并计划在 v7.0 移除。 | [ESP-IDF 6.0 peripheral migration guide](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32/migration-guides/release-6.x/6.0/peripherals.html) |
| 正确但需版本限定 | `stable` 文档可能比工程基线更新 | 2026-07-22 的 Espressif stable 页面显示最新 bugfix 为 6.0.2，而工程由 PlatformIO 7.0.1 固定在 6.0.1。代码示例应优先链接 `/v6.0.1/`，`stable` 只用于了解当前补丁状态。 | [ESP-IDF v6.0.1 docs](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/)、[ESP-IDF stable docs](https://docs.espressif.com/projects/esp-idf/en/stable/) |
| 正确 | C6 支持 2.4 GHz Wi-Fi 6、Bluetooth LE 和 IEEE 802.15.4 | 与 Seeed 和 Espressif 的型号资料一致。协议栈、角色、认证仍需项目级实现，仓库没有把射频能力等同于完整 Zigbee/Thread/Matter 产品，这一边界正确。 | [Seeed XIAO ESP32C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/)、[ESP-IDF ESP32-C6 Programming Guide](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32c6/) |
| 正确 | 精确版本声明可用于固定 PlatformIO 依赖 | PlatformIO 文档将纯版本号定义为 exact version；`espressif32@7.0.1` 的固定方式合理。 | [PlatformIO package version requirements](https://docs.platformio.org/en/latest/core/userguide/pkg/cmd_install.html#version-specifications) |
| 正确 | MkDocs 1.6.1 支持 `exclude_docs`，`mkdocs build --strict` 可将 warning 作为失败 | MkDocs 官方配置文档说明 `exclude_docs` 自 1.5 引入，且 `--strict` 会使 warning 失败。它主要验证本地文档、导航和锚点诊断，不等价于在线探测所有外部 URL。 | [MkDocs configuration](https://www.mkdocs.org/user-guide/configuration/)、[MkDocs command line interface](https://www.mkdocs.org/user-guide/cli/) |
| 需修正 | `theme.language: zh` 可实现中英双语站 | 该字段只设置单个项目的规范语言和主题字符串。Material 官方建议每种语言使用一个子目录项目，再用 `extra.alternate` 互链。页面正文仍需人工维护对应英文文件。 | [Material: Changing the language](https://squidfunk.github.io/mkdocs-material/setup/changing-the-language/) |

## 双语文档站验收要求 / Bilingual site acceptance criteria

Material 官方资料给出的关键边界是“一份 `mkdocs.yml` 对应一个规范语言”。无论最终采用两个配置文件、配置继承，还是第三方 i18n 插件，都应至少满足下列可观察结果：

Material's first-party guidance establishes that one `mkdocs.yml` has one canonical language. Whether the implementation uses two configs, configuration inheritance, or a third-party i18n plugin, the result should satisfy these observable requirements:

1. 中文和英文分别生成独立 URL，例如 `/zh/...` 与 `/en/...`，或默认中文 `/...` 加英文 `/en/...`。
2. 每个 HTML 文档的 `lang` 与正文语言一致；不能让英文正文继续声明 `lang=zh`。
3. 语言选择器使用绝对路径，并为链接输出正确 `hreflang`。
4. 同一路径存在中英文对应页时，切换语言应尽量停留在同一主题页面。
5. 中英文导航、站名、站点描述和搜索索引分别本地化；只翻译导航而保留中文正文不算双语完成。
6. CI 必须分别构建两个语言版本，并对两边的相对链接、锚点和导航遗漏执行严格检查。
7. 技术参数应使用同一事实源；英文版不能重新“自由翻译”引脚、单位、API 名或证据等级。

官方实现参考：

- [Material: canonical site language and per-language projects](https://squidfunk.github.io/mkdocs-material/setup/changing-the-language/#site-language)
- [Material: language selector](https://squidfunk.github.io/mkdocs-material/setup/changing-the-language/#site-language-selector)
- [MkDocs: configuration inheritance](https://www.mkdocs.org/user-guide/configuration/#configuration-inheritance)
- [MkDocs: link and navigation validation](https://www.mkdocs.org/user-guide/configuration/#validation)

## 建议修正顺序 / Recommended correction order

1. 全局把 S3 Sense microSD 的 GPIO21 修正为 GPIO3，并更新冲突说明。
2. 把 C6 CPU 描述改为“HP 单核 + LP RISC-V core”，或明确表格只描述主应用 CPU。
3. 在 S3 Sense 中英页面明确列出 OV2640、OV3660 与可替换 OV5640 的批次/兼容边界。
4. 建立逐页中英对应文件和双语 URL；不要只翻译 Material UI 或导航。
5. 对中英文站分别运行仓库文档验证器和 `mkdocs build --strict`。
6. 发布后检查真实 Pages URL、语言切换、canonical / `hreflang`、搜索结果和代表性深层链接。

## 一手来源清单 / First-party source register

以下来源均于 2026-07-22 检索：

1. Seeed Studio XIAO ESP32C3 Getting Started：板卡规格、D0–D10、strapping pins、电池和无用户 LED 边界。
2. Seeed Studio XIAO ESP32S3 Series Getting Started：基础版/Sense/Plus 规格、D0–D10、Sense SD 引脚、相机批次、Plus 额外引脚。
3. Seeed Studio XIAO ESP32S3 with FreeRTOS：8 MB Octal PSRAM、GPIO21 active-low 用户 LED、原生 ESP-IDF 示例。
4. Seeed Studio XIAO ESP32C6 Getting Started：HP/LP RISC-V 处理器、无线能力、D0–D10、GPIO15 用户灯。
5. Espressif ESP32-C6 Datasheet：ESP32-C6 处理器架构和芯片能力。
6. ESP-IDF v6.0.1 / v6.0 migration guides：USB Serial/JTAG、I²C driver 生命周期和版本化 API。
7. PlatformIO Espressif32 7.0.1 release、Registry 和 board pages：ESP-IDF 6.0.1 映射、Board ID、Flash 与 framework 支持。
8. MkDocs 1.6 configuration / CLI：`exclude_docs`、`validation` 和 strict build 语义。
9. Material for MkDocs language guide：规范语言、每语言一项目、语言选择器和 `hreflang`。

## 未被这些来源证明的事项 / What these sources do not prove

- 某块实物板已经运行当前工作区或当前 `main` 的固件。
- S3/C6 用户 LED 在当前固件中确实持续闪烁；官方资料只证明连接和控制极性，运行行为仍需串口与实物验证。
- GitHub Pages 当前部署内容与本地工作区相同；需要发布后的 URL 与 commit / artifact 证据。
- 外围模块在具体接线、供电和总线负载下工作正常。
- 第三方 MkDocs i18n 插件的行为；本记录按要求没有使用第三方插件文档作为事实来源。
