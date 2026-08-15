#pragma once

// ── 配置模板：复制为 src/config.h 并填写真实值 ──────────────────────
// src/config.h 已在 .gitignore 中，不会进入版本库

// 1 = 离线演示数据，不连接 Wi-Fi/iKuai；0 = 使用下面的真实配置
#define APP_DEMO_MODE 1

#define APP_WIFI_SSID "YOUR_SSID"
#define APP_WIFI_PASS "YOUR_PASSWORD"

#define APP_LAT  "39.9042"
#define APP_LONG "116.4074"

#define APP_TZ "CST-8"

#define APP_WEATHER_MIN 10

#define APP_BL_PCT 30

// ── iKuai 路由器监视 ───────────────────────────────────────────────
// token 获取：~/.ikuai-cli/config.json（ikuai-cli auth set-token）
#define IKUAI_HOST  "https://192.0.2.1"
#define IKUAI_TOKEN "REPLACE_WITH_IKUAI_TOKEN"
