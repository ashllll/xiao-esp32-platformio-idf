# 开发环境与官方示例

## 先选择正确的工程

| 路线 | 厂商要求 | 官方包内容 | 本仓库关系 |
|---|---|---|---|
| Arduino | `esp32 by Espressif Systems` ≥ 3.0.0 | `LVGL_Arduino`、`LVGL_Image`，LVGL 8.3.10、PNGdec 1.0.2 | 仅作板级参考，不能直接混入纯 ESP-IDF |
| 原生 ESP-IDF | ESP-IDF ≥ 5.3.1；下载包工程声明 5.3 | `ESP32-C6-LCD-1.47-Test`，LVGL、LCD、TF、RGB、Wi-Fi | 可作为驱动移植来源，但需按本工程 ESP-IDF 6.0.1 API 审查 |
| 本仓库 PlatformIO | `espressif32@7.0.1` / ESP-IDF 6.0.1 | 当前仅有 XIAO C3/S3/C6 board environments | 必须新增专用 board environment 后才能编译/烧录本板 |

Waveshare 下载包约 60 MB，解压后约 268 MB，包含完整依赖副本、示例图片和测试固件。不要把整个压缩包或派生构建物提交到本仓库；保留官方 URL、版本、SHA-256（如需长期冻结）和经过审查的最小驱动即可。

## 原生 ESP-IDF 工作流

在厂商工程目录中使用与示例匹配的 ESP-IDF：

```bash
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/cu.usbmodemXXXX flash monitor
```

烧录会覆盖设备现有固件。首次验证前先保存当前固件或确认厂商测试 BIN 可恢复。无法下载时进入 ROM 下载模式：按住 BOOT，短按 RESET，释放 RESET，最后释放 BOOT。

官方 `app_main()` 的意图是初始化无线、Flash 信息、RGB、TF 卡、LCD、LVGL UI，再循环调用 `lv_timer_handler()`。实际移植应检查每一步返回值，避免示例中的 `ESP_ERROR_CHECK` 把可恢复的 TF 卡缺失变成整机重启。

### 共享 SPI 的安全顺序

1. 建立一次 `SPI2_HOST` bus：SCLK 7、MOSI 6、MISO 5。
2. 注册 LCD device：CS 14、D/C 15、12 MHz、SPI mode 0。
3. 注册 TF device：CS 4，从低频初始化再按卡能力提升。
4. 确保未选中的 CS 为高；所有访问通过同一 bus 仲裁。
5. 初始化 LCD/reset/背光，背光先为 0，画面稳定后渐升至不高于 50%。
6. LVGL 任务和文件系统任务不得并发绕过 SPI bus/device API。

## Arduino

厂商包固定 LVGL 8.3.10 和 PNGdec 1.0.2，且网页明确警告 LVGL 8/9 驱动不兼容。打开示例前把包内库按 Arduino 离线库方式安装，并选择通用 `ESP32C6 Dev Module`，而不是网页旧模板中误写的 `ESP32S3 Dev Module`。

`LVGL_Arduino` 显示 Flash、SD 和无线扫描；`LVGL_Image` 从 TF 卡根目录轮播不大于 172×320 的 PNG。图片像素限制不等于文件大小、颜色格式或解码内存无限制。

## LVGL 与内存

官方 ESP-IDF 示例是 LVGL 8 API，并以 `172×20` 像素局部缓冲刷新。迁移到 LVGL 9 时需要按 LVGL 官方迁移说明更新 display、flush、tick 和 task handler API，不能只修改版本号。UI 任务应：

- 每 5–10 ms 提供 tick，按 LVGL 返回的建议周期调用 handler；
- 只在 LVGL 所属任务或受互斥保护的路径操作对象；
- DMA 缓冲使用合适能力内存，并检查分配失败；
- 屏幕、TF 卡和 Wi-Fi 同时工作时记录最低 free heap 与最大连续块。

## 测试固件

官方包的 `Firmware/ESP32-C6-LCD-1.47-Test.bin` 只适合恢复/板载功能初检。单个 BIN 的烧录地址必须以 Waveshare 的固件烧录说明或包内 flash 参数为准，不能猜测为 `0x0`。恢复出厂会覆盖现有应用和配置。
