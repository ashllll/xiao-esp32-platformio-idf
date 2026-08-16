#pragma once

#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t flash_size_bytes;
    uint32_t psram_size_bytes;
} board_support_diagnostics_t;

/* Return the human-readable name of the fixed board profile. */
const char *board_support_name(void);

/* Verify the XIAO ESP32-S3 8 MiB Flash / 8 MiB Octal PSRAM profile. */
esp_err_t board_support_validate(board_support_diagnostics_t *diagnostics);

#ifdef __cplusplus
}
#endif
