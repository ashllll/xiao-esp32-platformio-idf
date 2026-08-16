# ESP32-C6-LCD-1.47 iKuai monitor

This is the privacy-safe source for the 320×172 UniFi-LCM-style iKuai monitor
demo. It uses ESP-IDF 6.0.1, PlatformIO `espressif32@7.0.1`, LVGL 9.2.2,
crisp custom bitmap fonts, and a critically damped 30 FPS scrolling curve.

Small-label rendering uses the native-size 12 px/1 bpp font, while the large
numeric readout uses a 36 px/2 bpp font. The build-tested helper in
[`src/crisp_text.c`](src/crisp_text.c) keeps labels at integer pixel positions
with full text opacity and no runtime scaling. See the documentation site's
“Crisp text and blur troubleshooting” page for the complete checklist.

The public source and prebuilt release contain **no Wi-Fi password, router
address, API token, or router certificate**. The prebuilt image runs in offline
demo mode with synthetic data.

## Build the offline demo

```bash
cd examples/waveshare_esp32_c6_lcd_1_47_monitor
./scripts/fetch_lvgl.sh
cp src/config.example.h src/config.h
cp src/ikuai_cert.example.h src/ikuai_cert.h
pio run -e esp32-c6-lcd
```

`src/config.example.h` defaults to `APP_DEMO_MODE 1`, so no network connection
is made. To upload to a connected board:

```bash
pio run -e esp32-c6-lcd -t upload
pio device monitor -b 115200
```

Flashing overwrites the installed firmware. The supplied layout uses only the
first 4 MB and is intended for the non-touch ST7789 board. Verify the board
variant before flashing; do not use it on the JD9853 touch model.

## Enable real iKuai data

Edit the ignored local file `src/config.h`:

```c
#define APP_DEMO_MODE 0
#define APP_WIFI_SSID "YOUR_SSID"
#define APP_WIFI_PASS "YOUR_PASSWORD"
#define IKUAI_HOST  "https://YOUR_IKUAI_ADDRESS"
#define IKUAI_TOKEN "YOUR_IKUAI_API_TOKEN"
#define APP_BL_PCT 30
```

Keep `APP_BL_PCT` at or below 40. Export the certificate presented by the iKuai
HTTPS endpoint, then convert it to the ignored header:

```bash
python3 scripts/pem_to_header.py ikuai.pem src/ikuai_cert.h
```

Never commit `src/config.h`, `src/ikuai_cert.h`, a real token, or a personalized
firmware binary. Both private headers are covered by this example's
`.gitignore`.

## 中文说明

公开源码和预编译固件均不含 Wi-Fi、爱快地址、Token 或私有证书。直接复制
两个 `*.example.h` 后编译，默认是离线演示模式。要连接真实爱快，请在本地
`src/config.h` 中把 `APP_DEMO_MODE` 改为 `0`，填写 Wi-Fi、`IKUAI_HOST` 和
`IKUAI_TOKEN`，并将爱快 HTTPS 证书转换为 `src/ikuai_cert.h`。这两个真实
配置文件均已加入 `.gitignore`，不要提交个性化固件二进制。

完整烧录说明和发布包见文档站的“iKuai 监控示例固件”页面。
小屏字体模糊的原因、排查顺序和可复制代码见“清晰字体与模糊排查”页面。
