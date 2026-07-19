# 文档维护

文档与固件一起评审、构建和发布。目标是让另一位开发者只依靠仓库和明确的外部索引，就能复现环境、接线、构建、烧录和故障定位。

## 信息放在哪里

| 内容 | 位置 |
|---|---|
| 稳定操作步骤、引脚、API 和验证标准 | `docs/` |
| 板级宏和 D0–D10 映射 | `include/xiao_pins.h` |
| 组件公共 API | `components/<device>/include/` |
| 依赖和配置 | `platformio.ini`、`sdkconfig.*`、`idf_component.yml` |
| 大型 PDF、CAD、图片镜像、历史工程 | `XIAO_ESP32_REFERENCE_ROOT`（默认 `~/ESP32_资料整理`） |
| 版本变化 | `CHANGELOG.md` 和发布说明 |

项目文档不复制完整官方站点，也不把 `.platformio` 缓存当资料源提交。

## 新外设页面

```bash
cp docs/peripherals/_template.md docs/peripherals/<device>.md
```

完成模板所有字段，把页面加入 `mkdocs.yml`，然后运行严格构建。不要发布仍含未替换模板字段、空来源、未知供电或“待测试”却没有明确状态的页面。

## 来源优先级

1. 当前板卡/模块官方原理图、数据手册和 Seeed Wiki。
2. 当前 ESP-IDF/组件版本官方 API 文档与示例。
3. 本项目实测记录。
4. 本地历史归档和第三方文章。

发生冲突时保留冲突说明、板型/批次和核对日期，不静默覆盖。历史资料可帮助找到线索，但不能证明当前硬件或 API。

## 页面元数据

硬件和外设页面至少记录：

- 适用板卡与变体。
- 模块型号/版本和实际测试状态。
- SDK/组件版本。
- 官方 URL。
- 核对日期。
- 已知冲突和未验证范围。

## 链接与构建

```bash
python3 scripts/validate_docs.py
.venv/bin/mkdocs build --strict
```

文档验证不访问网络，负责检查 `project-baseline.json` 与依赖版本、已知旧基线、相对链接和锚点、MkDocs 导航覆盖、模板占位符及个人绝对路径。CI 对 pull request 运行该检查和严格构建，main 分支构建成功后发布 GitHub Pages。内部链接优先使用相对路径；外部链接使用官方永久入口，不链接搜索结果页。

外部 URL 会变化，不在每次 CI 中批量访问。维护版本或引用来源时应人工打开官方链接，并记录核对日期。

## 评审清单

- [ ] 命令从工程根目录可执行。
- [ ] 示例使用 ESP-IDF 6.0 API，不混入 Arduino API。
- [ ] 引脚同时给出 XIAO 丝印和各板 GPIO。
- [ ] 电气信息包含供电、逻辑电平和峰值电流。
- [ ] 依赖固定版本且许可证清晰。
- [ ] “编译通过”和“硬件已验证”没有混为一谈。
- [ ] 失败行为、超时、重试和恢复有说明。
- [ ] 来源、版本/板型和核对日期齐全。
- [ ] `python3 scripts/validate_docs.py` 通过。
- [ ] `mkdocs build --strict` 通过。

## 定期更新

升级 PlatformIO/ESP-IDF、换板卡批次、修改引脚或更换外设组件时，搜索并更新所有相关页面：

```bash
rg '7\.0\.1|ESP-IDF 6\.0\.1|XIAO_D|GPIO|<component-name>' docs README.md
```

平台升级时同步修改 `project-baseline.json`、`platformio.ini`、依赖清单、配置文档、资料索引和 `CHANGELOG.md`。更新后必须重新构建三个固件环境并执行严格文档构建。只有文档文字变化且不涉及配置/API/引脚时，才可只运行文档验证。
