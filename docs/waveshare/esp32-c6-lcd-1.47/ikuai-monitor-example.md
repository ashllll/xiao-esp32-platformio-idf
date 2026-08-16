# iKuai 监控示例固件

这个示例把 ESP32-C6-LCD-1.47 做成 320×172 的 UniFi LCM 风格网络状态屏，
显示 WAN 状态、上下行速率、在线设备数、Ping 和约 10 秒三色趋势曲线。

## 下载

- [v1.0.0 发布页](https://github.com/ashllll/xiao-esp32-platformio-idf/releases/tag/esp32-c6-lcd-monitor-v1.0.0)
- [脱敏源码](https://github.com/ashllll/xiao-esp32-platformio-idf/tree/main/examples/waveshare_esp32_c6_lcd_1_47_monitor)

发布包包含：

- `merged-flash.bin`：从地址 `0x0` 写入的一体化镜像；
- `bootloader.bin`、`partitions.bin`、`firmware.bin`：分离镜像；
- `flash_args`、`manifest.json` 和 `SHA256SUMS`；
- 发布包 README。

预编译镜像为**离线演示模式**，使用合成数据展示字体和曲线，不连接网络，
也不包含 Wi-Fi 密码、路由器地址、API Token 或爱快证书。

小屏字体若出现发虚、灰边或缩放模糊，请按[清晰字体与模糊排查](font-rendering.md)
检查原生分辨率、字体 bpp、标签坐标和 RGB565 字节序；该页也提供了可直接
复用且参与固件编译的 LVGL 代码示例。

## 烧录预编译演示

先确认设备是非触摸 ST7789 版本，不要写入使用 JD9853 的触摸版本。烧录会
覆盖当前固件和配置。

```bash
python3 -m esptool --chip esp32c6 \
  --port /dev/cu.usbmodemXXXX \
  write_flash 0x0 merged-flash.bin
```

发布包采用只使用前 4 MB 的布局，可覆盖已记录的 4 MB/8 MB 非触摸批次；
仍应先用 `esptool flash_id` 核对实际硬件。背光在驱动中硬限制为不高于 40%。

## 修改为真实爱快数据

进入源码目录后：

```bash
./scripts/fetch_lvgl.sh
cp src/config.example.h src/config.h
cp src/ikuai_cert.example.h src/ikuai_cert.h
```

编辑不会提交到 Git 的 `src/config.h`：

```c
#define APP_DEMO_MODE 0
#define APP_WIFI_SSID "你的 Wi-Fi 名称"
#define APP_WIFI_PASS "你的 Wi-Fi 密码"
#define IKUAI_HOST  "https://你的爱快地址"
#define IKUAI_TOKEN "你的 API Token"
#define APP_BL_PCT 30
```

再把爱快 HTTPS 证书转换为本地头文件：

```bash
python3 scripts/pem_to_header.py ikuai.pem src/ikuai_cert.h
pio run -e esp32-c6-lcd
```

`src/config.h`、`src/ikuai_cert.h`、真实 Token 和个性化编译的 BIN 均不得提交。
公开仓库只保存模板和无凭据的离线演示包。

## 验证边界

真实配置版已在 ESP32-C6FH8 实板上完成 LCD、Wi-Fi、IP 和连续曲线统计验证，
曲线实测约 26–27 FPS。公开 v1.0.0 离线镜像完成了干净目录编译和凭据扫描；
它没有在发布前再次写入实板，因此不能把“可构建”描述成独立的实物验收。
