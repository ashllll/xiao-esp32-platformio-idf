#pragma once
#include "esp_err.h"
#include <stdint.h>

// 板载 RGB 灯珠（WS2812，GPIO8）
esp_err_t ws2812_init(void);
void ws2812_set_rgb(uint8_t r, uint8_t g, uint8_t b);
