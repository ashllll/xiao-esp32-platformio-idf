#pragma once

#include <stdint.h>
#include <stdbool.h>

// iKuai 路由器实时监视数据（https + Bearer token + 固定证书）
// - system 端点 1s 轮询（CPU/内存/在线数/实时上下行速率）
// - ICMP ping 网关 1s 采样（延迟曲线）
// - 60 点曲线缓存（1 秒/点，实时滚动窗口）

#define IKUAI_CURVE_MAX 60

typedef struct {
    bool ok;
    uint32_t ts;
    float cpu_pct;
    float mem_pct;
    uint32_t online_cnt;
    uint32_t conn_cnt;
    uint32_t down_bps;      // 实时下行 B/s
    uint32_t up_bps;        // 实时上行 B/s
} ikuai_sys_t;

typedef struct {
    bool ok;
    uint32_t ts;
    int head;                          // 下一个写入位置
    int n;                             // 已填充点数（<= 60）
    float ping_ms[IKUAI_CURVE_MAX];    // 网关延迟 ms
    uint32_t down[IKUAI_CURVE_MAX];    // B/s
    uint32_t up[IKUAI_CURVE_MAX];      // B/s
} ikuai_curve_t;

void ikuai_monitor_start(void);
bool ikuai_get_sys(ikuai_sys_t *out);
bool ikuai_get_curve(ikuai_curve_t *out);
bool ikuai_recently_ok(void);
