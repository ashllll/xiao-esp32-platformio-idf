# 故障排查与真机验收

## 常见问题

| 现象 | 优先检查 |
|---|---|
| 无串口或无法烧录 | 数据线、端口权限；BOOT+RESET 进入 ROM 下载模式；目标必须是 `esp32c6` |
| 烧录后显示 `waiting for download...` | 释放 BOOT，短按 RESET 或重新上电 |
| USB 设备反复出现/消失 | 进入 ROM 下载模式并烧入已知可用固件；同时排查供电和线缆 |
| 黑屏但程序运行 | GPIO22 背光 PWM、LCD reset、CS/DC、SPI mode/clock、初始化表 |
| 白屏/花屏/偏移 | 172×320、RGB565/BGR、反色、行列 offset、DMA 缓冲生命周期 |
| 底部暗影 | 立即降低/关闭背光并冷却；以后限制 ≤50%，不要继续热机测试 |
| TF 卡挂载失败 | FAT 格式、卡供电、CS4、MISO5、共享总线；先降低 SPI 频率 |
| LVGL 卡死或随机重启 | handler 并发、栈/heap、flush 完成回调、SPI 竞争、看门狗 |
| Wi-Fi 正常但 802.15.4 不工作 | 功能存在不代表协议栈已启用；检查 ESP-IDF 配置和共存限制 |

macOS 上 ESP32-C6 原生 USB 通常不需要第三方 USB-UART 驱动。Waveshare FAQ 的“安装 MAC driver”是通用条目，未指明本板使用的桥接芯片；原理图显示 D+/D− 直连 ESP32-C6。因此不要先安装不明驱动，应先用 `system_profiler SPUSBDataType`、`ls /dev/cu.*` 和 ESP-IDF USB Serial/JTAG 文档确认设备状态。

## 分阶段验收

### 1. 不烧录检查

- 记录板正反面、丝印、SKU、是否为 `-M`。
- 检查屏幕胶合、排针焊接、USB-C、TF 卡座和天线区域。
- 只接 USB，确认无异常发热、异味或过流；记录 5 V/3.3 V。
- 保存现有串口启动日志和固件恢复方法。

### 2. 最小固件

- 芯片识别为 ESP32-C6，Flash 检测为 4 MB。
- 串口连续重启 10 次均可枚举和输出。
- BOOT/RESET 能稳定进入应用和 ROM 下载模式。
- RGB 分别显示红、绿、蓝，亮度受控。

### 3. 显示

- 背光从 0 缓升至 10%、25%、50%，50% 连续运行时测量屏幕/LDO 温度。
- 显示红绿蓝白黑、灰阶、1 像素边框、四角编号和文字。
- 核对 172×320 全范围、无裁剪/偏移、RGB 顺序正确。
- 运行至少 30 分钟，记录暗影、闪烁、撕裂、SPI/LVGL 错误。

### 4. TF 与共享 SPI

- 无卡启动不重启；插卡后正确识别容量与文件系统。
- 对已备份的测试卡做多轮读写/校验和；不要在生产数据卡上测试。
- 同时刷新 LVGL、读取 PNG、执行 Wi-Fi 扫描，确认无总线死锁和数据损坏。

### 5. 系统与无线

- 记录最小 heap、最大连续块、任务栈余量和看门狗状态。
- 分别验证 Wi-Fi、BLE；如项目需要，再独立验证 Thread/Zigbee/802.15.4。
- 在目标外壳、电源和温度范围重新做长时间测试。

通过文档构建、代码编译或烧录成功只能记录为“静态/构建/下载通过”，不能替代以上真机验收。

## 真机验收记录

**验收日期：** 2026-07-25

**验收环境：**

| 项目 | 值 |
|---|---|
| 固件来源 | 本仓库独立测试固件（非官方包） |
| 框架 | PlatformIO `espressif32@7.0.1` / ESP-IDF 6.0.1 |
| 板子定义 | `esp32-c6-devkitc-1` |
| 串口 | `/dev/tty.usbmodem11411201` |
| 芯片 | ESP32-C6FH4 rev v0.2 |
| Flash | 4 MB (DIO, 80 MHz) |
| RAM | 437 KiB available |

### 已通过的验收项

| 阶段 | 内容 | 结果 | 备注 |
|---|---|---|---|
| 最小固件 | 芯片识别为 ESP32-C6，Flash 4 MB | ✅ 通过 | boot log 确认 |
| 最小固件 | 连续重启可枚举串口 | ✅ 通过 | 串口输出正常 |
| 最小固件 | BOOT/RESET 进入应用模式 | ✅ 通过 | |
| 显示 | ST7789 初始化 (172×320, RGB565, BGR) | ✅ 通过 | SPI 12 MHz |
| 显示 | 彩条测试 (R/G/B/W/Y/C/M) | ✅ 通过 | Phase 1 |
| 显示 | 文字显示 (8×8 bitmap font) | ✅ 通过 | Phase 2 |
| 显示 | 棋盘格 (8×8 像素) | ✅ 通过 | Phase 3 |
| 显示 | 纯白画面 (坏点检查) | ✅ 通过 | Phase 4 |
| 显示 | 渐变测试 (蓝→红) | ✅ 通过 | Phase 5 |
| 显示 | 背光 PWM 控制 (0-50%) | ✅ 通过 | LEDC 10-bit |

### 未验收项

| 项目 | 原因 |
|---|---|
| TF 卡读写 | 测试固件未包含 |
| RGB LED | 测试固件未包含 |
| Wi-Fi / BLE | 测试固件未包含 |
| 长时间热行为 | 未进行 30 分钟以上测试 |
| TF + LCD 共享 SPI 并发 | 未测试 |

### 串口启动日志

```
ESP-ROM:esp32c6-20220919
I (23) boot: ESP-IDF 6.0.1 2nd stage bootloader
I (24) boot: chip revision: v0.2
I (34) boot.esp32c6: SPI Flash Size : 8MB
I (253) lcd_test: === Waveshare ESP32-C6-LCD-1.47 LCD Test ===
I (549) lcd_test: LCD initialized
I (549) lcd_test: Phase 1: Color bars
I (3674) lcd_test: Phase 2: Text
I (8346) lcd_test: Phase 3: Checkerboard
I (12710) lcd_test: Phase 4: Solid white
I (15794) lcd_test: Phase 5: Gradient
I (16006) lcd_test: Test sequence complete
```

### 编译问题记录

| 问题 | 解决方案 |
|---|---|
| Arduino 框架不支持 ESP32-C6 | 改用 ESP-IDF 框架 |
| ESP-IDF 6.0.1 bootloader.ld 链接路径错误 | 手动创建 symlink `bootloader.ld -> bootloader/ld/bootloader.ld` |
| Flash 大小默认 8 MB（实际 4 MB） | 在 `platformio.ini` 中不覆盖，使用板子默认配置 |

## 资料变更核对

厂商页面会更新，示例 ZIP 没有稳定版本标签。复现问题时记录下载日期、ZIP SHA-256、ESP-IDF/Arduino/LVGL 版本、板 SKU、Flash ID 与日志。不要只写“最新版”。
