#pragma once

#include "esp_err.h"
#include <stdint.h>

// TF 卡（microSD，SPI 模式，与 LCD 共享 SPI2 总线）
// - CS=GPIO4, MISO=GPIO5, MOSI/SCLK 与 LCD 共用 GPIO6/7
// - 低频(1MHz)起步挂载，无卡/坏卡不阻塞启动、不重启、绝不格式化
typedef enum {
    TF_NONE = 0,     // 无卡或挂载失败
    TF_MOUNTING,     // 挂载中
    TF_OK,           // 已挂载可写
} tf_state_t;

void     tf_card_start(void);             // 创建挂载任务（不阻塞应用启动）
tf_state_t tf_card_get_state(void);
uint32_t tf_card_get_size_mb(void);       // 卡容量（MB）
void     tf_append_log(const char *line); // 追加一行到 /sdcard/weather.log（自动加换行）
