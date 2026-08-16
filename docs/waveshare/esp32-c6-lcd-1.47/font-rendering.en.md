# Crisp text and blur troubleshooting

The ESP32-C6-LCD-1.47 panel is physically 172×320. A landscape UI should let
LVGL create a native 320×172 display instead of rendering to a smaller canvas
and enlarging it. Refresh rate affects motion continuity; it does not sharpen
static text.

## Rendering choices used by this example

- `LV_COLOR_DEPTH` is 16 and the display buffer uses native RGB565.
- Small labels use the final-size `ui_font_crisp_12`; 1 bpp gives deliberate,
  hard pixel edges.
- Large numbers use `ui_font_crisp_36`; 2 bpp balances outline smoothness and
  Flash usage at the larger size.
- Labels stay on integer pixel coordinates. No label, parent, canvas, or
  screenshot is scaled at runtime.
- Text color and opacity are explicit so a translucent inherited style cannot
  create a gray haze.
- RGB565 bytes are swapped exactly once before the ST7789 transfer.

LVGL fonts store glyph bitmaps. Higher bpp provides more opacity levels for
smoother outlines but consumes more storage. This example therefore generates
the 12 px and 36 px fonts separately instead of scaling one font at runtime.
See the [LVGL 9.2 font documentation](https://docs.lvgl.io/9.2/overview/font.html)
and [RGB565 byte-order guidance](https://docs.lvgl.io/9.2/porting/display.html).

## Build-tested example

The complete helper is in
[`src/crisp_text.c`](https://github.com/ashllll/xiao-esp32-platformio-idf/blob/main/examples/waveshare_esp32_c6_lcd_1_47_monitor/src/crisp_text.c)
and is compiled by the current PlatformIO project.

```c
LV_FONT_DECLARE(ui_font_crisp_12);

lv_obj_t *label = ui_crisp_label_create(
    lv_screen_active(),
    12,                         /* integer x pixel */
    8,                          /* integer y pixel */
    &ui_font_crisp_12,          /* generated at the final 12 px size */
    0xF4F7FB                    /* high-contrast text */
);
lv_label_set_text(label, "WAN ONLINE");
```

The helper itself keeps the rendering state explicit:

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

## Diagnostic order

1. Lift one corner of the protective film to rule out haze and reflections.
2. Confirm `lv_display_create(320, 172)`. Do not stretch a 160×86 or portrait
   framebuffer into the landscape panel.
3. Inspect the font headers: the small font should say `Size: 12 px / Bpp: 1`
   and the numeric font `Size: 36 px / Bpp: 2`. Do not scale the label again.
4. Set text to `LV_OPA_COVER` and test bright text on a solid dark background.
5. Display one-pixel horizontal/vertical lines and a checkerboard. If those are
   also fuzzy, investigate the panel, viewing angle, film, and LCD init values.
6. If strokes have color fringes, check for a missing or duplicated RGB565 byte
   swap. LVGL 9 offers `lv_draw_sw_rgb565_swap()`; do not use it in addition to
   an existing custom swap.
7. Take a fixed-focus macro photo. A photo rescaled by a browser, gallery, or
   chat service is not pixel-level acceptance evidence.

## Validation boundary

These settings and the helper compile and remove software-side scaling,
inherited-opacity, and byte-order ambiguity. A documentation or firmware build
still cannot prove that a specific physical panel is sharp. Final acceptance
requires static text and a one-pixel test pattern on the target board, inspected
directly or from the original macro image.
