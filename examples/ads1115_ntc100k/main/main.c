#include <inttypes.h>

#include "ads1115.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ntc_thermistor.h"
#include "xiao_board.h"
#include "xiao_pins.h"

static const char *TAG = "ads1115_ntc";

/* Replace these values with the exact NTC and fixed-resistor specifications. */
static const ntc_beta_config_t NTC_CONFIG = {
    .nominal_resistance_ohm = 100000.0,
    .beta_kelvin = 3950.0,
    .nominal_temperature_c = 25.0,
    .fixed_resistor_ohm = 100000.0,
};

static esp_err_t create_i2c_bus(i2c_master_bus_handle_t *bus)
{
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = XIAO_I2C_SDA,
        .scl_io_num = XIAO_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };
    return i2c_new_master_bus(&bus_config, bus);
}

void app_main(void)
{
    xiao_board_diagnostics_t diagnostics;
    ESP_ERROR_CHECK(xiao_board_validate(&diagnostics));
    ESP_ERROR_CHECK(xiao_board_init());
    ESP_LOGI(TAG, "XIAO_RUNTIME_READY version=1 board=%s", xiao_board_name());

    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(create_i2c_bus(&bus));

    ads1115_t adc;
    const ads1115_config_t adc_config = ADS1115_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(ads1115_init(&adc, bus, &adc_config));

    ESP_LOGI(TAG,
             "ADS1115 ready: address=0x%02X AIN0=divider AIN1=3V3 R25=%.0f beta=%.0f",
             adc_config.i2c_address,
             NTC_CONFIG.nominal_resistance_ohm,
             NTC_CONFIG.beta_kelvin);

    while (true) {
        int16_t supply_raw;
        int16_t node_raw;
        double supply_voltage;
        double node_voltage;

        esp_err_t err = ads1115_read_single_ended(
            &adc, ADS1115_CHANNEL_AIN1, &supply_raw, &supply_voltage);
        if (err == ESP_OK) {
            err = ads1115_read_single_ended(
                &adc, ADS1115_CHANNEL_AIN0, &node_raw, &node_voltage);
        }

        double resistance_ohm;
        double temperature_c;
        if (err == ESP_OK &&
            ntc_low_side_temperature(supply_voltage, node_voltage, &NTC_CONFIG,
                                     &resistance_ohm, &temperature_c)) {
            ESP_LOGI(TAG,
                     "supply_raw=%" PRId16 " supply=%.6fV node_raw=%" PRId16
                     " node=%.6fV ntc=%.1fOhm temperature=%.2fC",
                     supply_raw, supply_voltage, node_raw, node_voltage,
                     resistance_ohm, temperature_c);
        } else if (err != ESP_OK) {
            ESP_LOGE(TAG, "ADS1115 read failed: %s", esp_err_to_name(err));
        } else {
            ESP_LOGE(TAG,
                     "invalid divider sample: supply=%.6fV node=%.6fV; check open/short wiring",
                     supply_voltage, node_voltage);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
