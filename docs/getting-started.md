# 快速开始

## 选择开发板

默认是 XIAO ESP32C3。可以使用 `-e` 临时选择目标：

```bash
pio run -e xiao_esp32c3
pio run -e xiao_esp32s3
pio run -e xiao_esp32c6
```

也可以修改 `platformio.ini`：

```ini
[platformio]
default_envs = xiao_esp32s3
```

## 本地验证

```bash
pio run
mkdocs build --strict
```

PlatformIO 会把工具链和构建结果保存在 `.pio/`，不会污染系统 ESP-IDF 环境。

