#pragma once

#include <stdint.h>

// Waveshare ESP32-C6-LCD-1.47 — ST7789 172x320 SPI 屏（LVGL 底层）
// 横屏显示：320 宽 x 172 高（MADCTL 旋转 90 度）
#define LCD_W 320
#define LCD_H 172

void lcd_init(void);
void lcd_set_backlight(uint8_t pct);   // 硬上限 40%

// 推送一个矩形区域到屏幕；data 为 RGB565 字节流（MSB-first，即高字节在前），
// 每行从左到右连续。内部按 16 行分块规避 C6 SPI 单次传输上限。
void lcd_flush_area(int x0, int y0, int x1, int y1, const void *data);
