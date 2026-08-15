// 板载 RGB 灯珠驱动（WS2812 类，GPIO8，RMT 10 MHz 分辨率）
#include "ws2812.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include "esp_log.h"
#include "esp_check.h"

#define RGB_GPIO 8
#define RMT_RES  10000000   // 10 MHz -> 100 ns/刻度

static rmt_channel_handle_t s_chan;
static rmt_encoder_handle_t s_enc;

esp_err_t ws2812_init(void) {
    rmt_tx_channel_config_t ch = {
        .gpio_num = RGB_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RMT_RES,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_RETURN_ON_ERROR(rmt_new_tx_channel(&ch, &s_chan), "ws2812", "tx channel failed");

    // WS2812 时序（100ns 刻度）：0=0.4us 高+0.8us 低，1=0.8us 高+0.4us 低
    rmt_bytes_encoder_config_t enc = {
        .bit0 = { .duration0 = 4, .level0 = 1, .duration1 = 8, .level1 = 0 },
        .bit1 = { .duration0 = 8, .level0 = 1, .duration1 = 4, .level1 = 0 },
    };
    ESP_RETURN_ON_ERROR(rmt_new_bytes_encoder(&enc, &s_enc), "ws2812", "bytes encoder failed");

    ESP_RETURN_ON_ERROR(rmt_enable(s_chan), "ws2812", "enable failed");
    return ESP_OK;
}

void ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t grb[3] = { g, r, b };   // WS2812 字节序 GRB
    rmt_transmit_config_t cfg = { .loop_count = 0 };
    if (rmt_transmit(s_chan, s_enc, grb, sizeof(grb), &cfg) == ESP_OK)
        rmt_tx_wait_all_done(s_chan, 100);
}
