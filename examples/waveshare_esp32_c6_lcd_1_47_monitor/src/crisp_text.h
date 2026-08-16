#pragma once

#include <stdint.h>

#include "lvgl.h"

/**
 * Create a label for native-resolution small LCD rendering.
 *
 * The caller supplies integer pixel coordinates and a font generated at the
 * final display size. Do not apply image/canvas zoom to the returned label.
 */
lv_obj_t *ui_crisp_label_create(lv_obj_t *parent,
                                int32_t x,
                                int32_t y,
                                const lv_font_t *font,
                                uint32_t rgb);
