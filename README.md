# XIAO ESP32 PlatformIO 模板

面向 Seeed Studio XIAO ESP32C3、ESP32S3 和 ESP32C6 的个人项目模板：

- PlatformIO 管理 ESP-IDF、工具链、依赖、编译和烧录。
- MkDocs Material 管理硬件、固件、引脚及外围设备文档。
- GitHub Actions 验证全部板卡环境并构建文档站。

完整本地资料库位于 `/Users/llll/ESP32_资料整理`；工程文档的“资料索引”页记录了经过核对的官方入口和本地资料用法。

## 快速开始

```bash
pio run -e xiao_esp32c3
pio run -e xiao_esp32c3 -t upload
pio device monitor -b 115200
```

启动文档站：

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements-docs.txt
mkdocs serve
```

默认环境在 `platformio.ini` 的 `default_envs` 中修改。

## 工程结构

- `src/main.c`：应用入口，演示 `xiao_board` 组件（LED 心跳 + 环境自检日志）。
- `components/xiao_board/`：板级支持组件；新增外设驱动放入 `components/<device>/`。
- `include/xiao_pins.h`：XIAO 丝印 D0–D10 到各板 GPIO 的符号映射，业务代码不写裸 GPIO。
- `docs/`：文档站源码；新增外设页从 `docs/peripherals/_template.md` 复制。

版本与变更记录见 `CHANGELOG.md`；许可证为 Apache-2.0（`LICENSE`）。
