# Contributing

欢迎提交 XIAO ESP32C3、ESP32S3 和 ESP32C6 的代码、文档与硬件验证改进。请保持变更小而可验证。

## 开始前

- 使用 `platformio.ini` 和 `project-baseline.json` 中固定的版本，不单独替换 ESP-IDF framework 包。
- 使用 `XIAO_D0`–`XIAO_D10` 等符号，不在业务代码散布裸 GPIO。
- 新外设必须记录模块版本、供电、逻辑电平、接线、总线、依赖、来源和实际验证范围。
- 不提交凭据、证书私钥、个人串口路径、构建目录或本地资料库内容。

## 本地验证

```bash
python3 scripts/validate_docs.py
python3 -m unittest discover -s tests -v
pio run -e xiao_esp32c3
pio run -e xiao_esp32s3
pio run -e xiao_esp32c6
mkdocs build --strict
```

没有对应开发板时，可以报告构建结果，但不能声称真机通过。硬件结果应按[真机验证矩阵](docs/hardware/validation.md)记录，烧录必须得到设备所有者明确授权。

## 提交内容

- 说明目标板卡、变体和受影响的组件或文档。
- 列出实际运行的命令及结果。
- 对硬件改动附接线、供电和串口验收证据；不要附带秘密信息。
- 依赖或版本升级必须同步更新 `project-baseline.json`、文档、变更记录和全部板卡构建。
