// iKuai 路由器实时监视：HTTPS API + ICMP ping
// - system 端点 1s 轮询：实时上下行速率、CPU/内存/在线数
// - ping 网关 1s 采样：延迟曲线
// - 60 点环形缓存，供 UI 画实时滚动曲线
#include "ikuai_monitor.h"
#include "ikuai_cert.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_timer.h"
#include "apps/ping/ping_sock.h"   // IDF 6.0 无 esp_ping 组件，用 lwip ping_sock
#include "esp_netif.h"

static const char *TAG = "ikuai";

#define API_BASE IKUAI_HOST
#define HTTP_TIMEOUT_MS 5000

static SemaphoreHandle_t s_mux;
static ikuai_sys_t    s_sys;
static ikuai_curve_t  s_curve;
static volatile uint32_t s_last_ok_ts = 0;
static volatile float s_latest_ping_ms = -1;   // ping 回调只存最新值

// ─── HTTP（复用 client，IDF 6.0.1 每次 init/cleanup 泄漏 ~5KB）───

typedef struct { char *buf; int len; int cap; } resp_t;
static esp_http_client_handle_t s_client = NULL;
static resp_t s_resp;

static esp_err_t on_http_evt(esp_http_client_event_t *evt) {
    if (evt->event_id == HTTP_EVENT_ON_DATA) {
        resp_t *r = (resp_t *)evt->user_data;
        int n = evt->data_len;
        if (r->len + n < r->cap) {
            memcpy(r->buf + r->len, evt->data, n);
            r->len += n;
            r->buf[r->len] = 0;
        }
    }
    return ESP_OK;
}

static bool api_get(const char *path, char *buf, int cap) {
    char url[200];
    snprintf(url, sizeof(url), "%s%s", API_BASE, path);
    if (!s_client) {
        esp_http_client_config_t cfg = {
            .url = API_BASE "/api/v4.0/monitoring/system",
            .method = HTTP_METHOD_GET,
            .timeout_ms = HTTP_TIMEOUT_MS,
            .cert_pem = ikuai_cert_pem,
            .skip_cert_common_name_check = true,
            .event_handler = on_http_evt,
            .user_data = &s_resp,
            .user_agent = "ESP32-C6-LCD-1.47/1.0",
        };
        s_client = esp_http_client_init(&cfg);
        if (!s_client) return false;
        ESP_LOGI(TAG, "http client initialized once");
    }
    s_resp.buf = buf;
    s_resp.len = 0;
    s_resp.cap = cap;
    esp_http_client_set_url(s_client, url);
    char auth[80];
    snprintf(auth, sizeof(auth), "Bearer %s", IKUAI_TOKEN);
    esp_http_client_set_header(s_client, "Authorization", auth);
    esp_err_t err = esp_http_client_perform(s_client);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "http fail %s: %s", path, esp_err_to_name(err));
        return false;
    }
    return s_resp.len > 0;
}

// ─── 极简 JSON 提取 ─────────────────────────────────────────────────

static bool array_first_str(const char *s, const char *key, char *out, int cap) {
    char pat[32];
    snprintf(pat, sizeof(pat), "\"%s\":[\"", key);
    const char *p = strstr(s, pat);
    if (!p) return false;
    p += strlen(pat);
    int i = 0;
    while (*p && *p != '"' && i < cap - 1) out[i++] = *p++;
    out[i] = 0;
    return i > 0;
}

static bool kv_num_range(const char *start, const char *end, const char *key, double *out) {
    char pat[32];
    snprintf(pat, sizeof(pat), "\"%s\":", key);
    const char *p = strstr(start, pat);
    if (!p || p > end) return false;
    p += strlen(pat);
    while (*p == ' ') p++;
    char *endp;
    double v = strtod(p, &endp);
    if (endp == p || endp > end) return false;
    *out = v;
    return true;
}

static bool kv_str_range(const char *start, const char *end, const char *key,
                         char *out, int cap) {
    char pat[32];
    snprintf(pat, sizeof(pat), "\"%s\":\"", key);
    const char *p = strstr(start, pat);
    if (!p || p > end) return false;
    p += strlen(pat);
    int i = 0;
    while (*p && *p != '"' && i < cap - 1) out[i++] = *p++;
    out[i] = 0;
    return i > 0;
}

// 解析 system 响应，写入 sys 缓存 + 曲线缓存
static void parse_system(const char *resp) {
    ikuai_sys_t s = { 0 };
    char tmp[16];
    double v;
    if (array_first_str(resp, "cpu", tmp, sizeof(tmp))) s.cpu_pct = (float)atof(tmp);
    const char *mem = strstr(resp, "\"memory\":{");
    if (mem) {
        if (kv_str_range(mem, mem + 300, "used", tmp, sizeof(tmp)))
            s.mem_pct = (float)atof(tmp);
    }
    const char *ou = strstr(resp, "\"online_user\":{");
    if (ou && kv_num_range(ou, ou + 200, "count", &v)) s.online_cnt = (uint32_t)v;
    const char *st = strstr(resp, "\"stream\":{");
    if (st) {
        if (kv_num_range(st, st + 300, "connect_num", &v)) s.conn_cnt = (uint32_t)v;
        if (kv_num_range(st, st + 300, "download", &v)) s.down_bps = (uint32_t)v;
        if (kv_num_range(st, st + 300, "upload", &v)) s.up_bps = (uint32_t)v;
    }
    s.ok = s.down_bps > 0 || s.up_bps > 0 || s.online_cnt > 0;
    s.ts = (uint32_t)(esp_timer_get_time() / 1000000);
    if (s.ok) s_last_ok_ts = s.ts;

    xSemaphoreTake(s_mux, portMAX_DELAY);
    s_sys = s;
    // 速率 + ping 统一进曲线（1s 采样，由本函数推进槽位）
    int h = s_curve.head;
    s_curve.down[h] = s.down_bps;
    s_curve.up[h] = s.up_bps;
    s_curve.ping_ms[h] = s_latest_ping_ms;
    if (s_curve.n < IKUAI_CURVE_MAX) s_curve.n++;
    s_curve.head = (h + 1) % IKUAI_CURVE_MAX;
    s_curve.ts = s.ts;
    xSemaphoreGive(s_mux);
}

// ─── Ping（ICMP 网关，1s 采样）──────────────────────────────────────

static esp_ping_handle_t s_ping = NULL;

static void on_ping_success(esp_ping_handle_t hdl, void *args) {
    uint32_t t = 0;
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &t, sizeof(t));
    s_latest_ping_ms = (float)t;
}

static void on_ping_timeout(esp_ping_handle_t hdl, void *args) {
    s_latest_ping_ms = -1;
}

static void ping_start(void) {
    ip_addr_t gw;
    esp_netif_ip_info_t ipi;
    esp_netif_t *sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta && esp_netif_get_ip_info(sta, &ipi) == ESP_OK) {
        ip_addr_copy_from_ip4(gw, ipi.gw);     // 用实际网关
    } else {
        IP_ADDR4(&gw, 192, 168, 9, 1);
    }
    esp_ping_config_t cfg = ESP_PING_DEFAULT_CONFIG();
    cfg.target_addr = gw;
    cfg.count = ESP_PING_COUNT_INFINITE;   // 持续 ping（1s 间隔实时曲线）
    cfg.interval_ms = 1000;                // 1s 采样
    cfg.timeout_ms = 800;
    esp_ping_callbacks_t cbs = {
        .on_ping_success = on_ping_success,
        .on_ping_timeout = on_ping_timeout,
    };
    if (esp_ping_new_session(&cfg, &cbs, &s_ping) == ESP_OK) {
        esp_ping_start(s_ping);
        ESP_LOGI(TAG, "ping started (1s interval)");
    }
}

// ─── 轮询任务 ───────────────────────────────────────────────────────

static void poll_task(void *arg) {
    enum { BUF_SZ = 4096 };
    char *buf = (char *)malloc(BUF_SZ);
    if (!buf) { vTaskDelete(NULL); return; }
    ping_start();
    uint32_t next = 0;
    for (;;) {
        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000000);
        if (now >= next) {
            next = now + 1;   // 1s 实时采样
            if (api_get("/api/v4.0/monitoring/system", buf, BUF_SZ))
                parse_system(buf);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// ─── 对外接口 ───────────────────────────────────────────────────────

void ikuai_monitor_start(void) {
    s_mux = xSemaphoreCreateMutex();
    if (!s_mux) return;
    xTaskCreate(poll_task, "ikuai", 8192, NULL, 4, NULL);
    ESP_LOGI(TAG, "ikuai monitor started");
}

bool ikuai_get_sys(ikuai_sys_t *out) {
    if (!s_mux) return false;
    xSemaphoreTake(s_mux, portMAX_DELAY);
    *out = s_sys;
    xSemaphoreGive(s_mux);
    return out->ok;
}

bool ikuai_get_curve(ikuai_curve_t *out) {
    if (!s_mux) return false;
    xSemaphoreTake(s_mux, portMAX_DELAY);
    *out = s_curve;
    xSemaphoreGive(s_mux);
    return out->n > 0;
}

bool ikuai_recently_ok(void) {
    if (!s_last_ok_ts) return false;
    return (uint32_t)(esp_timer_get_time() / 1000000) - s_last_ok_ts < 10;
}
