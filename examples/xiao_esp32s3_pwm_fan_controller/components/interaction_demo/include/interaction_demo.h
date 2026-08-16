#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool running;
    uint8_t target_percent;
    uint8_t displayed_percent;
} interaction_demo_snapshot_t;

/* Start the library-backed encoder input and SH1106/LVGL demo. */
esp_err_t interaction_demo_start(void);

/* Read a coherent snapshot for serial diagnostics. */
void interaction_demo_get_snapshot(interaction_demo_snapshot_t *snapshot);

#ifdef __cplusplus
}
#endif
