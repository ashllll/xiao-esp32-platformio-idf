// iKuai 实时监视小屏（UniFi LCM 风格单页）
// 首页只保留 WAN 状态、实时上下行、在线设备、PING 和约 10 秒三色趋势。
// 每个刷新帧固定左移 1px，并用临界阻尼连续跟随新采样，避免低速走格与折线突变。
#include "desktop_widget.h"
#include "lcd_driver.h"
#include "ws2812.h"
#include "config.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "ikuai_monitor.h"
#include "esp_heap_caps.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_timer.h"

LV_FONT_DECLARE(ui_font_crisp_12);
LV_FONT_DECLARE(ui_font_crisp_36);

#ifndef APP_DEMO_MODE
#define APP_DEMO_MODE 0
#endif

static const char *TAG = "widget";

#define BIT_IP (1 << 0)
static EventGroupHandle_t s_eg;
static char s_ip[16] = "---";
static volatile bool s_wifi_ok = false;

#define CLR_BG       0x03070B
#define CLR_PANEL    0x091018
#define CLR_BORDER   0x172330
#define CLR_GRID     0x13202A
#define CLR_TEXT     0xF4F7FA
#define CLR_DIM      0x718092
#define CLR_GREEN    0x45D39A
#define CLR_DOWN     0x50E3C2
#define CLR_UP       0x6AA8FF
#define CLR_PING     0xFFB454
#define CLR_RED      0xFF5F67

#define CURVE_W 298
#define CURVE_H 45
#define CURVE_FPS 30
#define CURVE_SMOOTH_SEC 0.28f

#define RGB565(r,g,b) (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

// ─── Wi-Fi ───────────────────────────────────────────────────────────

#if !APP_DEMO_MODE
static void on_wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        s_wifi_ok = false;
        xEventGroupClearBits(s_eg, BIT_IP);
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)data;
        esp_ip4addr_ntoa(&e->ip_info.ip, s_ip, sizeof(s_ip));
        s_wifi_ok = true;
        xEventGroupSetBits(s_eg, BIT_IP);
        ESP_LOGI(TAG, "got ip %s", s_ip);
    }
}

static void wifi_start(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wc = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wc));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi_event, NULL));
    wifi_config_t cfg = { 0 };
    strncpy((char *)cfg.sta.ssid, APP_WIFI_SSID, sizeof(cfg.sta.ssid) - 1);
    strncpy((char *)cfg.sta.password, APP_WIFI_PASS, sizeof(cfg.sta.password) - 1);
    cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
}
#endif

// ═══════════════════ 流动曲线引擎 ═════════════════════════════════════

static lv_obj_t *canvas_curve;
LV_DRAW_BUF_DEFINE_STATIC(cvs_curve, CURVE_W, CURVE_H, LV_COLOR_FORMAT_RGB565);

typedef struct {
    float cur;
    float target;
    float velocity;
    uint16_t color;
    uint16_t dim;
} curve_track_t;

enum { TRACK_DOWN, TRACK_UP, TRACK_PING, TRACK_COUNT };
static curve_track_t s_track[TRACK_COUNT];
static uint32_t s_peak_down = 1024;
static uint32_t s_peak_up = 1024;
static float s_peak_ping = 50.0f;
static uint16_t s_c_bg;
static uint16_t s_c_grid;
static int s_last_y[TRACK_COUNT] = { CURVE_H - 3, CURVE_H - 3, CURVE_H - 3 };
static uint32_t s_curve_frames;

static uint16_t mix565(uint16_t bg, uint16_t fg, uint8_t a) {
    if (a == 0) return bg;
    if (a >= 254) return fg;
    int r = (((fg >> 11) & 31) * a + ((bg >> 11) & 31) * (255 - a)) / 255;
    int g = (((fg >> 5) & 63) * a + ((bg >> 5) & 63) * (255 - a)) / 255;
    int b = ((fg & 31) * a + (bg & 31) * (255 - a)) / 255;
    return (uint16_t)((r << 11) | (g << 5) | b);
}

static void curve_clear(void) {
    uint16_t *buf = (uint16_t *)cvs_curve.data;
    for (int y = 0; y < CURVE_H; y++) {
        uint16_t c = (y == CURVE_H / 3 || y == CURVE_H * 2 / 3) ? s_c_grid : s_c_bg;
        for (int x = 0; x < CURVE_W; x++) buf[y * CURVE_W + x] = c;
    }
}

static int curve_y(float value) {
    if (value < 0) value = 0;
    if (value > 1) value = 1;
    return (int)((1.0f - value) * (CURVE_H - 5) + 2.5f);
}

static void curve_clear_column(uint16_t *buf, int x) {
    for (int y = 0; y < CURVE_H; y++) {
        buf[y * CURVE_W + x] =
            (y == CURVE_H / 3 || y == CURVE_H * 2 / 3) ? s_c_grid : s_c_bg;
    }
}

static void curve_draw_track(uint16_t *buf, int x, int index, const curve_track_t *track) {
    int y = curve_y(track->cur);
    int y0 = s_last_y[index];
    int lo = y < y0 ? y : y0;
    int hi = y > y0 ? y : y0;
    for (int yy = lo; yy <= hi; yy++) buf[yy * CURVE_W + x] = track->dim;
    buf[y * CURVE_W + x] = track->color;
    if (y + 1 < CURVE_H) buf[(y + 1) * CURVE_W + x] = track->color;
    if (x > 0) buf[y * CURVE_W + x - 1] = track->dim;
    s_last_y[index] = y;
}

// 临界阻尼平滑：位置和速度都连续，目标每秒跳变时也不会出现折线硬拐角。
static void curve_smooth_step(curve_track_t *track) {
    const float dt = 1.0f / CURVE_FPS;
    const float omega = 2.0f / CURVE_SMOOTH_SEC;
    const float x = omega * dt;
    const float decay = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    const float current = track->cur;
    const float change = current - track->target;
    const float temp = (track->velocity + omega * change) * dt;

    track->velocity = (track->velocity - omega * temp) * decay;
    track->cur = track->target + (change + temp) * decay;

    // 目标反向变化时禁止越过目标，避免视觉上的虚假过冲。
    if (((track->target - current) > 0.0f) == (track->cur > track->target)) {
        track->cur = track->target;
        track->velocity = 0.0f;
    }
    if (track->cur < 0.0f) {
        track->cur = 0.0f;
        track->velocity = 0.0f;
    } else if (track->cur > 1.0f) {
        track->cur = 1.0f;
        track->velocity = 0.0f;
    }
}

// 30fps 固定推进 1px：298px 宽对应约 9.9 秒，运动节奏不会因累计取整而顿挫。
static void curve_roll(void) {
    uint16_t *buf = (uint16_t *)cvs_curve.data;
    for (int i = 0; i < TRACK_COUNT; i++) {
        curve_smooth_step(&s_track[i]);
    }

    for (int y = 0; y < CURVE_H; y++) {
        uint16_t *row = buf + y * CURVE_W;
        memmove(row, row + 1, (CURVE_W - 1) * sizeof(uint16_t));
    }
    curve_clear_column(buf, CURVE_W - 1);

    for (int i = 0; i < TRACK_COUNT; i++) {
        curve_draw_track(buf, CURVE_W - 1, i, &s_track[i]);
    }
    s_curve_frames++;
    lv_obj_invalidate(canvas_curve);
}

// ═══════════════════ UI ══════════════════════════════════════════════

static lv_obj_t *lbl_status, *lbl_online, *lbl_down, *lbl_up;
static lv_obj_t *lbl_down_unit, *lbl_up_unit, *lbl_ping, *lbl_ip;
static lv_obj_t *dot_status;

static void fmt_rate(uint32_t bps, char *value, int value_cap, const char **unit) {
    if (bps >= 1048576) {
        float v = bps / 1048576.0f;
        snprintf(value, value_cap, v >= 100 ? "%.0f" : "%.1f", v);
        *unit = "MB/s";
    } else if (bps >= 1024) {
        float v = bps / 1024.0f;
        snprintf(value, value_cap, v >= 100 ? "%.0f" : "%.1f", v);
        *unit = "KB/s";
    } else {
        snprintf(value, value_cap, "%u", (unsigned)bps);
        *unit = "B/s";
    }
}

static lv_obj_t *mk_label(lv_obj_t *parent, int x, int y, const lv_font_t *f, uint32_t color) {
    lv_obj_t *l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, f, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
    lv_obj_set_pos(l, x, y);
    lv_obj_set_style_text_letter_space(l, 0, 0);
    lv_label_set_text(l, "");
    return l;
}

static lv_obj_t *mk_block(lv_obj_t *parent, int x, int y, int w, int h, uint32_t color) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    return o;
}

static void ui_create(void) {
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(CLR_BG), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    dot_status = lv_obj_create(scr);
    lv_obj_remove_style_all(dot_status);
    lv_obj_set_style_bg_color(dot_status, lv_color_hex(CLR_GREEN), 0);
    lv_obj_set_style_bg_opa(dot_status, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(dot_status, 3, 0);
    lv_obj_set_pos(dot_status, 12, 9);
    lv_obj_set_size(dot_status, 6, 6);

    lbl_status = mk_label(scr, 23, 3, &ui_font_crisp_12, CLR_TEXT);
    lv_label_set_text(lbl_status, "WAN ONLINE");

    lbl_online = mk_label(scr, 0, 3, &ui_font_crisp_12, CLR_DIM);
    lv_label_set_text(lbl_online, "-- CLIENTS");
    lv_obj_align(lbl_online, LV_ALIGN_TOP_MID, 0, 3);

    lbl_ping = mk_label(scr, 0, 3, &ui_font_crisp_12, CLR_PING);
    lv_label_set_text(lbl_ping, "PING -- MS");
    lv_obj_align(lbl_ping, LV_ALIGN_TOP_RIGHT, -12, 3);

    mk_block(scr, 12, 23, 3, 11, CLR_DOWN);
    lv_obj_t *down_name = mk_label(scr, 21, 20, &ui_font_crisp_12, CLR_DIM);
    lv_label_set_text(down_name, "DOWNLOAD");

    mk_block(scr, 172, 23, 3, 11, CLR_UP);
    lv_obj_t *up_name = mk_label(scr, 181, 20, &ui_font_crisp_12, CLR_DIM);
    lv_label_set_text(up_name, "UPLOAD");

    lbl_down = mk_label(scr, 12, 34, &ui_font_crisp_36, CLR_TEXT);
    lbl_down_unit = mk_label(scr, 116, 54, &ui_font_crisp_12, CLR_DOWN);
    lbl_up = mk_label(scr, 172, 34, &ui_font_crisp_36, CLR_TEXT);
    lbl_up_unit = mk_label(scr, 276, 54, &ui_font_crisp_12, CLR_UP);
    lv_label_set_text(lbl_down, "--");
    lv_label_set_text(lbl_down_unit, "MB/s");
    lv_label_set_text(lbl_up, "--");
    lv_label_set_text(lbl_up_unit, "MB/s");
    lv_obj_align_to(lbl_down_unit, lbl_down, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -6);
    lv_obj_align_to(lbl_up_unit, lbl_up, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -6);

    mk_block(scr, 159, 24, 1, 47, CLR_BORDER);

    lv_obj_t *graph = lv_obj_create(scr);
    lv_obj_remove_style_all(graph);
    lv_obj_set_pos(graph, 10, 82);
    lv_obj_set_size(graph, 300, 74);
    lv_obj_set_style_bg_color(graph, lv_color_hex(CLR_PANEL), 0);
    lv_obj_set_style_bg_opa(graph, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(graph, lv_color_hex(CLR_BORDER), 0);
    lv_obj_set_style_border_width(graph, 1, 0);
    lv_obj_set_style_radius(graph, 8, 0);
    lv_obj_set_style_clip_corner(graph, true, 0);

    lv_obj_t *trend = mk_label(graph, 9, 2, &ui_font_crisp_12, CLR_DIM);
    lv_label_set_text(trend, "10S LIVE");
    lv_obj_t *legend_down = mk_label(graph, 101, 2, &ui_font_crisp_12, CLR_DOWN);
    lv_label_set_text(legend_down, "DOWN");
    lv_obj_t *legend_up = mk_label(graph, 165, 2, &ui_font_crisp_12, CLR_UP);
    lv_label_set_text(legend_up, "UP");
    lv_obj_t *legend_ping = mk_label(graph, 209, 2, &ui_font_crisp_12, CLR_PING);
    lv_label_set_text(legend_ping, "PING");

    canvas_curve = lv_canvas_create(graph);
    lv_obj_set_pos(canvas_curve, 1, 23);
    lv_canvas_set_draw_buf(canvas_curve, &cvs_curve);

    lbl_ip = mk_label(scr, 12, 157, &ui_font_crisp_12, CLR_DIM);
    lv_label_set_text(lbl_ip, APP_DEMO_MODE ? "IP DEMO" : "IP ---");
    lv_obj_t *source = mk_label(scr, 0, 157, &ui_font_crisp_12, CLR_DIM);
    lv_label_set_text(source, APP_DEMO_MODE ? "DEMO DATA" : "IKUAI LIVE");
    lv_obj_align(source, LV_ALIGN_TOP_RIGHT, -12, 157);
}

// 每秒：更新数字 + 采样目标
static void ui_update(void) {
#if APP_DEMO_MODE
    static uint32_t tick;
    tick++;

    // 三组不同周期的三角波，离线即可检查数字、字体和曲线动画。
    uint32_t d_phase = tick % 18;
    uint32_t u_phase = (tick + 5) % 14;
    uint32_t p_phase = (tick + 3) % 10;
    uint32_t d_tri = d_phase <= 9 ? d_phase : 18 - d_phase;
    uint32_t u_tri = u_phase <= 7 ? u_phase : 14 - u_phase;
    uint32_t p_tri = p_phase <= 5 ? p_phase : 10 - p_phase;
    uint32_t down_bps = (2U + d_tri * 3U) * 1024U * 1024U;
    uint32_t up_bps = (1U + u_tri) * 384U * 1024U;
    float ping_ms = 12.0f + (float)(p_tri * 7U);

    char d[16], u[16], line[32];
    const char *du, *uu;
    fmt_rate(down_bps, d, sizeof(d), &du);
    fmt_rate(up_bps, u, sizeof(u), &uu);
    lv_label_set_text(lbl_down, d);
    lv_label_set_text(lbl_up, u);
    lv_label_set_text(lbl_down_unit, du);
    lv_label_set_text(lbl_up_unit, uu);
    lv_obj_align_to(lbl_down_unit, lbl_down, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -6);
    lv_obj_align_to(lbl_up_unit, lbl_up, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -6);
    lv_label_set_text(lbl_online, "18 CLIENTS");
    lv_label_set_text(lbl_status, "DEMO ONLINE");
    lv_obj_set_style_bg_color(dot_status, lv_color_hex(CLR_GREEN), 0);
    snprintf(line, sizeof(line), "PING %.0f MS", ping_ms);
    lv_label_set_text(lbl_ping, line);
    lv_label_set_text(lbl_ip, "IP DEMO");

    s_track[TRACK_DOWN].target = (float)d_tri / 9.0f;
    s_track[TRACK_UP].target = (float)u_tri / 7.0f;
    s_track[TRACK_PING].target = ping_ms / 47.0f;
    return;
#endif

    ikuai_sys_t s;
    ikuai_curve_t c;

    if (ikuai_get_sys(&s) && s.ok) {
        char d[16], u[16];
        const char *du, *uu;
        fmt_rate(s.down_bps, d, sizeof(d), &du);
        fmt_rate(s.up_bps, u, sizeof(u), &uu);
        lv_label_set_text(lbl_down, d);
        lv_label_set_text(lbl_up, u);
        lv_label_set_text(lbl_down_unit, du);
        lv_label_set_text(lbl_up_unit, uu);
        lv_obj_align_to(lbl_down_unit, lbl_down, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -6);
        lv_obj_align_to(lbl_up_unit, lbl_up, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, -6);
        char line[32];
        snprintf(line, sizeof(line), "%lu CLIENTS", (unsigned long)s.online_cnt);
        lv_label_set_text(lbl_online, line);
        lv_label_set_text(lbl_status, "WAN ONLINE");
        lv_obj_set_style_bg_color(dot_status, lv_color_hex(CLR_GREEN), 0);

        if (s.down_bps > s_peak_down) s_peak_down = s.down_bps;
        else s_peak_down = (uint32_t)(((uint64_t)s_peak_down * 63 + s.down_bps) / 64);
        if (s_peak_down < 1024) s_peak_down = 1024;

        if (s.up_bps > s_peak_up) s_peak_up = s.up_bps;
        else s_peak_up = (uint32_t)(((uint64_t)s_peak_up * 63 + s.up_bps) / 64);
        if (s_peak_up < 1024) s_peak_up = 1024;

        s_track[TRACK_DOWN].target = (float)s.down_bps / (float)s_peak_down;
        s_track[TRACK_UP].target = (float)s.up_bps / (float)s_peak_up;
    } else {
        lv_label_set_text(lbl_down, "--");
        lv_label_set_text(lbl_up, "--");
        lv_label_set_text(lbl_down_unit, "");
        lv_label_set_text(lbl_up_unit, "");
        lv_label_set_text(lbl_online, "-- CLIENTS");
        lv_label_set_text(lbl_status, s_wifi_ok ? "WAN OFFLINE" : "WIFI OFFLINE");
        lv_obj_set_style_bg_color(dot_status, lv_color_hex(CLR_RED), 0);
        s_track[TRACK_DOWN].target = 0;
        s_track[TRACK_UP].target = 0;
    }

    if (ikuai_get_curve(&c) && c.n > 0) {
        int slot = (c.head - 1 + IKUAI_CURVE_MAX) % IKUAI_CURVE_MAX;
        float p = c.ping_ms[slot];
        if (p < 0) {
            lv_label_set_text(lbl_ping, "PING -- MS");
            s_track[TRACK_PING].target = 0;
        }
        else {
            char line[24];
            snprintf(line, sizeof(line), "PING %.0f MS", p);
            lv_label_set_text(lbl_ping, line);
            if (p > s_peak_ping) s_peak_ping = p;
            else s_peak_ping = s_peak_ping * 0.985f + p * 0.015f;
            if (s_peak_ping < 50.0f) s_peak_ping = 50.0f;
            s_track[TRACK_PING].target = p / s_peak_ping;
        }
    } else {
        lv_label_set_text(lbl_ping, "PING -- MS");
        s_track[TRACK_PING].target = 0;
    }

    static char s_last_ip[16] = "";
    if (strcmp(s_ip, s_last_ip) != 0) {
        strncpy(s_last_ip, s_ip, sizeof(s_last_ip));
        char line[32];
        snprintf(line, sizeof(line), "IP %s", s_ip);
        lv_label_set_text(lbl_ip, line);
    }
}

static lv_timer_t *s_ui_timer;
static lv_timer_t *s_roll_timer;

static void ui_timer_cb(lv_timer_t *t) {
    ui_update();

    // heap 监控（30s）
    static uint32_t s_last_heap_log = 0;
    static uint32_t s_min_heap = 0xFFFFFFFF;
    static uint32_t s_last_curve_frames = 0;
    uint32_t sec_now = (uint32_t)(esp_timer_get_time() / 1000000);
    if (sec_now - s_last_heap_log >= 30) {
        uint32_t elapsed = sec_now - s_last_heap_log;
        s_last_heap_log = sec_now;
        uint32_t free8 = (uint32_t)heap_caps_get_free_size(MALLOC_CAP_8BIT);
        size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
        uint32_t frame_delta = s_curve_frames - s_last_curve_frames;
        s_last_curve_frames = s_curve_frames;
        uint32_t fps_x10 = elapsed ? frame_delta * 10 / elapsed : 0;
        if (free8 < s_min_heap) s_min_heap = free8;
        ESP_LOGI(TAG, "heap free=%u min=%u largest=%u curve_fps=%u.%u",
                 (unsigned)free8, (unsigned)s_min_heap, (unsigned)largest,
                 (unsigned)(fps_x10 / 10), (unsigned)(fps_x10 % 10));
    }

#if APP_DEMO_MODE
    ws2812_set_rgb(30, 180, 60);
#else
    if (!s_wifi_ok)                     ws2812_set_rgb(180, 20, 20);
    else if (!ikuai_recently_ok())      ws2812_set_rgb(200, 110, 0);
    else                                ws2812_set_rgb(30, 180, 60);
#endif
}

static void roll_timer_cb(lv_timer_t *t) {
    curve_roll();
}

static void lv_tick_cb(void *arg) { lv_tick_inc(1); }

static void ui_task(void *arg) {
    lv_init();
    lv_port_disp_init();

    s_c_bg = RGB565(0x09, 0x10, 0x18);
    s_c_grid = RGB565(0x13, 0x20, 0x2A);
    s_track[TRACK_DOWN].color = RGB565(0x50, 0xE3, 0xC2);
    s_track[TRACK_UP].color = RGB565(0x6A, 0xA8, 0xFF);
    s_track[TRACK_PING].color = RGB565(0xFF, 0xB4, 0x54);
    for (int i = 0; i < TRACK_COUNT; i++) {
        s_track[i].dim = mix565(s_c_bg, s_track[i].color, 120);
    }

    ui_create();
    curve_clear();

    const esp_timer_create_args_t tmr_args = { .callback = lv_tick_cb, .name = "lv_tick" };
    esp_timer_handle_t tmr;
    ESP_ERROR_CHECK(esp_timer_create(&tmr_args, &tmr));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tmr, 1000));

    s_ui_timer = lv_timer_create(ui_timer_cb, 1000, NULL);
    s_roll_timer = lv_timer_create(roll_timer_cb, 1000 / CURVE_FPS, NULL);

    for (;;) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// ─── Entry ───────────────────────────────────────────────────────────

void widget_start(void) {
    s_eg = xEventGroupCreate();
    if (!s_eg) ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    ESP_ERROR_CHECK(ws2812_init());
#if APP_DEMO_MODE
    s_wifi_ok = true;
    strncpy(s_ip, "DEMO", sizeof(s_ip));
    ESP_LOGI(TAG, "offline demo mode; Wi-Fi and iKuai disabled");
#else
    wifi_start();
    ikuai_monitor_start();
#endif
    ESP_LOGI(TAG, "heap at start: free=%u largest=%u",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    xTaskCreate(ui_task, "ui", 8192, NULL, 6, NULL);
}
