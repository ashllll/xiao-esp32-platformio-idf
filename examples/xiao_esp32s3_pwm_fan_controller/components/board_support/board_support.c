#include "board_support.h"

#include <inttypes.h>

#include "board_pins.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_psram.h"

static const char *TAG = "board_support";

const char *board_support_name(void)
{
    return BOARD_NAME;
}

esp_err_t board_support_validate(board_support_diagnostics_t *diagnostics)
{
    if (diagnostics == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    *diagnostics = (board_support_diagnostics_t){0};

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    if (chip_info.model != CHIP_ESP32S3) {
        ESP_LOGE(TAG, "Chip model mismatch for %s: actual=%d expected=%d",
                 BOARD_NAME, chip_info.model, CHIP_ESP32S3);
        return ESP_ERR_INVALID_VERSION;
    }

    esp_err_t err = esp_flash_get_size(NULL, &diagnostics->flash_size_bytes);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Unable to read Flash size for %s: %s",
                 BOARD_NAME, esp_err_to_name(err));
        return err;
    }
    if (diagnostics->flash_size_bytes != BOARD_EXPECTED_FLASH_BYTES) {
        ESP_LOGE(TAG, "Flash size mismatch for %s: actual=%" PRIu32
                      " expected=%u",
                 BOARD_NAME, diagnostics->flash_size_bytes,
                 (unsigned)BOARD_EXPECTED_FLASH_BYTES);
        return ESP_ERR_INVALID_SIZE;
    }

    diagnostics->psram_size_bytes = (uint32_t)esp_psram_get_size();
    if (diagnostics->psram_size_bytes != BOARD_EXPECTED_PSRAM_BYTES) {
        ESP_LOGE(TAG, "PSRAM size mismatch for %s: actual=%" PRIu32
                      " expected=%u",
                 BOARD_NAME, diagnostics->psram_size_bytes,
                 (unsigned)BOARD_EXPECTED_PSRAM_BYTES);
        return ESP_ERR_INVALID_SIZE;
    }

    return ESP_OK;
}
