#include "crisp_text.h"

lv_obj_t *ui_crisp_label_create(lv_obj_t *parent,
                                int32_t x,
                                int32_t y,
                                const lv_font_t *font,
                                uint32_t rgb) {
    LV_ASSERT_NULL(parent);
    LV_ASSERT_NULL(font);

    lv_obj_t *label = lv_label_create(parent);

    /* Keep glyphs on the native pixel grid: use the final-size font directly. */
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(rgb), 0);
    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    lv_obj_set_style_opa(label, LV_OPA_COVER, 0);
    lv_label_set_text(label, "");

    return label;
}
