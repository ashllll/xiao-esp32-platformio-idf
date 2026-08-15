// LVGL → ST7789 显示适配层（横屏 320x172）
// - 双缓冲（320x40 两片，共 51.2KB）
// - RGB565 小端内存 → 大端字节流（ST7789 需要 MSB-first）
#include "lv_port_disp.h"
#include "lvgl.h"
#include "lcd_driver.h"

#define DRAW_BUF_ROWS 40

static lv_color_t s_buf1[LCD_W * DRAW_BUF_ROWS];
static lv_color_t s_buf2[LCD_W * DRAW_BUF_ROWS];

static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    int w = area->x2 - area->x1 + 1;
    int h = area->y2 - area->y1 + 1;

    // 小端 RGB565 -> MSB-first（ST7789 字节序）
    uint8_t *p = px_map;
    for (int i = 0; i < w * h; i++, p += 2) {
        uint8_t t = p[0];
        p[0] = p[1];
        p[1] = t;
    }

    lcd_flush_area(area->x1, area->y1, area->x2, area->y2, px_map);
    lv_display_flush_ready(disp);
}

void lv_port_disp_init(void) {
    lv_display_t *disp = lv_display_create(LCD_W, LCD_H);
    lv_display_set_flush_cb(disp, disp_flush_cb);
    lv_display_set_buffers(disp, s_buf1, s_buf2, sizeof(s_buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
}
