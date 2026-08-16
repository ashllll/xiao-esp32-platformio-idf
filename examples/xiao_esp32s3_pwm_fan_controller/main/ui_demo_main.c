#include <inttypes.h>

#include "board_pins.h"
#include "board_support.h"
#include "esp_err.h"
#include "esp_log.h"
#include "interaction_demo.h"

static const char *TAG = "ui_demo_main";

void app_main(void)
{
    board_support_diagnostics_t diagnostics;
    ESP_ERROR_CHECK(board_support_validate(&diagnostics));
    ESP_ERROR_CHECK(interaction_demo_start());

    ESP_LOGI(TAG, "Board profile: %s, flash=%" PRIu32 " MB, psram=%" PRIu32 " MB",
             board_support_name(), diagnostics.flash_size_bytes / (1024U * 1024U),
             diagnostics.psram_size_bytes / (1024U * 1024U));
    ESP_LOGI(TAG, "EC10_FAN_PWM_READY version=12 board=%s", BOARD_ID);
    ESP_LOGI(TAG, "Fan PWM enabled on D3; power switching, tachometer, Matter and NVS are disabled");
}
