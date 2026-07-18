# 工程结构

```text
src/                 应用入口（main.c：LED 心跳 + 环境自检）
include/             板级公共头文件（xiao_pins.h）
components/          ESP-IDF 自定义组件
  xiao_board/        板级支持：用户 LED 与板级信息 API（首个外设组件的参照）
docs/                项目文档（peripherals/_template.md 为外设页模板，不参与构建）
platformio.ini       开发板环境与构建配置
sdkconfig.defaults   可复现的 ESP-IDF 默认配置
sdkconfig.flash-4mb  C3/C6 的 4 MB Flash 配置
sdkconfig.xiao-esp32s3  S3 的 8 MB Flash 与 Octal PSRAM 配置
```

初始固件版本在根目录 `CMakeLists.txt` 中设置为 `0.1.0`，变更记录见 `CHANGELOG.md`。发布新版本时同步更新 `PROJECT_VER`、`CHANGELOG.md` 和 Git Tag。

外围设备驱动建议放入 `components/<device>/`，并复制 `docs/peripherals/_template.md` 建立 `docs/peripherals/<device>.md`。公共 API 放在组件的 `include/` 中。
