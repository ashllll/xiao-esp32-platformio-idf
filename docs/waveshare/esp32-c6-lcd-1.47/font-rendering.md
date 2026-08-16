# 清晰字体与模糊排查

ESP32-C6-LCD-1.47 的物理分辨率是 172×320；横屏界面应让 LVGL 直接创建
320×172 显示，不要先画到低分辨率画布再放大。刷新率决定动态连续性，不能
修复静止文字的模糊。

## 本示例采用的清晰渲染方案

- `LV_COLOR_DEPTH` 为 16，显示缓冲区使用原生 RGB565；
- 小标签使用最终尺寸生成的 `ui_font_crisp_12`，1 bpp 提供明确的像素边缘；
- 大数字使用 `ui_font_crisp_36`，2 bpp 在大字号轮廓和 Flash 占用之间折中；
- 标签只放在整数像素坐标，不对标签、父容器或截图做运行时缩放；
- 文字颜色和文字不透明度显式设置，避免继承半透明样式形成灰雾；
- ST7789 写入前只交换一次 RGB565 高低字节。

LVGL 字体本质上是字形位图。提高 bpp 会增加可用的不透明度级别，让轮廓更
平滑，同时也增加字体存储空间。这里刻意对 12 px 和 36 px 使用不同 bpp，
而不是用同一字体运行时缩放。参见 [LVGL 9.2 字体说明](https://docs.lvgl.io/9.2/overview/font.html)
和 [显示接口的 RGB565 字节序说明](https://docs.lvgl.io/9.2/porting/display.html)。

## 可直接复用的代码

完整实现位于
[`src/crisp_text.c`](https://github.com/ashllll/xiao-esp32-platformio-idf/blob/main/examples/waveshare_esp32_c6_lcd_1_47_monitor/src/crisp_text.c)，
并由当前 PlatformIO 工程实际编译。

```c
LV_FONT_DECLARE(ui_font_crisp_12);

lv_obj_t *label = ui_crisp_label_create(
    lv_screen_active(),
    12,                         /* 整数像素 x */
    8,                          /* 整数像素 y */
    &ui_font_crisp_12,          /* 按最终 12 px 生成，不缩放 */
    0xF4F7FB                    /* 高对比度文字颜色 */
);
lv_label_set_text(label, "WAN ONLINE");
```

核心辅助函数如下：

```c
lv_obj_t *ui_crisp_label_create(lv_obj_t *parent,
                                int32_t x,
                                int32_t y,
                                const lv_font_t *font,
                                uint32_t rgb) {
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(rgb), 0);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    lv_obj_set_style_opa(label, LV_OPA_COVER, 0);
    return label;
}
```

## 排查顺序

1. 撕掉或掀起屏幕保护膜的一角，排除保护膜雾化和表面反光。
2. 确认 `lv_display_create(320, 172)`，不要把 160×86、172×320 截图或画布
   拉伸到横屏。
3. 检查字体文件头：12 px 字体应为 `Size: 12 px / Bpp: 1`，大数字应为
   `Size: 36 px / Bpp: 2`；不要再给标签设置缩放。
4. 把文字设为 `LV_OPA_COVER`，用亮文字和纯色深背景排除低对比度造成的视觉
   发虚。
5. 显示 1 px 横线、竖线和棋盘格。如果线条也发虚，问题不在字体，继续检查
   面板批次、观察角度、保护膜和 LCD 初始化参数。
6. 如果颜色错位或笔画出现彩边，检查 RGB565 是否未交换或重复交换。LVGL 9
   也提供 `lv_draw_sw_rgb565_swap()`；使用它时应删除自定义的第二次交换。
7. 最后在固定焦距下拍摄微距照片。聊天软件、浏览器和相册缩放后的照片不能
   作为像素清晰度验收依据。

## 验证边界

这套设置和辅助函数能够通过编译并消除软件侧的字体缩放、透明度继承和字节序
歧义，但文档构建或固件编译不能证明某块实际屏幕已经清晰。最终仍需在目标板
上显示静态文字与 1 px 测试图，再以肉眼或原始微距照片验收。
