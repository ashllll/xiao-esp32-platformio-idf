# 真机验证矩阵

“能够编译”与“已经在真实开发板上运行”是两种不同证据。本页记录验证层级、当前覆盖和可复现步骤，避免把 CI 成功误写成硬件实测。

## 证据层级

| 层级 | 证据 | 能证明 | 不能证明 |
|---|---|---|---|
| 构建 | `pio run -e <environment>` | API、配置、链接和工具链可生成固件 | 供电、板载资源、串口和外围器件实际可用 |
| 交付包 | `manifest.json`、`SHA256SUMS` | 二进制版本、源码提交、烧录参数和文件完整性可追溯 | 固件已经写入某块开发板 |
| 运行时 | READY、容量自检和连续 heartbeat | 当前板上固件能启动且基础内存配置符合目标环境 | 板上固件一定来自当前工作区或某个指定提交 |
| 来源绑定 | 运行时输出版本/提交并与 manifest 对比 | 板上固件与指定交付包一致 | 外围器件和无线功能已经完成专项验证 |

当前 READY v1 只包含板型，不包含 Git 提交，因此运行时验证尚未达到“来源绑定”层级。发布说明必须如实区分这两者。

## 当前覆盖

| 环境 | CI 构建与打包 | 真机运行时 | 来源绑定 | 最近核对 |
|---|---|---|---|---|
| `xiao_esp32c3` | 已通过 | 未记录 | 未实现 | 2026-07-18 |
| `xiao_esp32s3` | 已通过 | 历史记录曾通过：8 MB Flash、8 MB PSRAM、READY v1、连续 10 个 heartbeat；当前待重验 | 未实现 | 2026-07-18 |
| `xiao_esp32c6` | 已通过 | 未记录 | 未实现 | 2026-07-18 |

S3 的历史只读验收没有执行烧录，因此不能单独证明设备运行的是当前 `main`。2026-07-18 二次复核时，连接设备运行的是其他项目固件，连续两次未出现本模板的 READY 标志；当前状态因此降级为“待重验”，不视为模板故障或本工程真机通过。C3/C6 只有构建和打包证据，不能标记为“真机已验证”。

## 不烧录验收

先列出端口；存在多个 Espressif 设备时必须明确指定目标：

```bash
pio device list
python3 scripts/verify_hardware.py \
  --environment xiao_esp32s3 \
  --port /dev/cu.usbmodemXXXX \
  --heartbeats 10 \
  --json-output
```

不带 `--flash` 时，脚本可以复位并读取设备，但不会写入持久存储。成功结果必须同时包含正确板型、Flash/PSRAM 容量、READY v1 和严格递增的 heartbeat。

## 与交付包绑定的验收

此流程会覆盖开发板固件，只能在用户明确授权后执行：

1. 从干净提交构建目标环境并生成交付包。
2. 保存 `dist/<environment>/manifest.json` 和 `SHA256SUMS`。
3. 明确目标端口，再运行带 `--flash` 的验证命令。
4. 保存 JSON 验收结果、板卡具体变体、供电方式和测试日期。
5. 在 READY 协议加入项目版本和源码提交后，将设备输出与 manifest 对比。

```bash
python3 scripts/verify_hardware.py \
  --environment xiao_esp32s3 \
  --port /dev/cu.usbmodemXXXX \
  --heartbeats 10 \
  --flash \
  --json-output
```

不要在 CI、自动化脚本或 Agent 工作流中默认增加 `--flash`。
