#include "ads1115.h"

#include <stddef.h>
#include <string.h>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ADS1115_REG_CONVERSION 0x00
#define ADS1115_REG_CONFIG 0x01

#define ADS1115_CONFIG_OS_START (1U << 15)
#define ADS1115_CONFIG_OS_READY (1U << 15)
#define ADS1115_CONFIG_MUX_AIN0_GND (4U << 12)
#define ADS1115_CONFIG_MODE_SINGLE_SHOT (1U << 8)
#define ADS1115_CONFIG_COMPARATOR_DISABLED 0x0003U

#define ADS1115_I2C_TIMEOUT_MS 50

static bool ads1115_config_is_valid(const ads1115_config_t *config)
{
    return config != NULL &&
           config->i2c_address >= 0x48 && config->i2c_address <= 0x4B &&
           config->scl_speed_hz > 0 && config->scl_speed_hz <= 400000 &&
           config->full_scale >= ADS1115_FSR_6_144V &&
           config->full_scale <= ADS1115_FSR_0_256V &&
           config->data_rate >= ADS1115_DATA_RATE_8_SPS &&
           config->data_rate <= ADS1115_DATA_RATE_860_SPS &&
           config->conversion_timeout_ms > 0;
}

static esp_err_t ads1115_write_register(ads1115_t *adc, uint8_t reg, uint16_t value)
{
    const uint8_t payload[] = {
        reg,
        (uint8_t)(value >> 8),
        (uint8_t)(value & 0xFFU),
    };
    return i2c_master_transmit(adc->device, payload, sizeof(payload), ADS1115_I2C_TIMEOUT_MS);
}

static esp_err_t ads1115_read_register(ads1115_t *adc, uint8_t reg, uint16_t *value)
{
    if (value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t payload[2];
    esp_err_t err = i2c_master_transmit_receive(
        adc->device, &reg, sizeof(reg), payload, sizeof(payload), ADS1115_I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        return err;
    }

    *value = ((uint16_t)payload[0] << 8) | payload[1];
    return ESP_OK;
}

double ads1115_raw_to_voltage(ads1115_fsr_t full_scale, int16_t raw_code)
{
    static const double full_scale_volts[] = {
        6.144,
        4.096,
        2.048,
        1.024,
        0.512,
        0.256,
    };

    if (full_scale < ADS1115_FSR_6_144V || full_scale > ADS1115_FSR_0_256V) {
        return 0.0;
    }
    return (double)raw_code * full_scale_volts[full_scale] / 32768.0;
}

esp_err_t ads1115_init(ads1115_t *adc,
                       i2c_master_bus_handle_t bus,
                       const ads1115_config_t *config)
{
    if (adc == NULL || bus == NULL || !ads1115_config_is_valid(config)) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(adc, 0, sizeof(*adc));
    const i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = config->i2c_address,
        .scl_speed_hz = config->scl_speed_hz,
    };
    esp_err_t err = i2c_master_bus_add_device(bus, &device_config, &adc->device);
    if (err != ESP_OK) {
        return err;
    }

    adc->full_scale = config->full_scale;
    adc->data_rate = config->data_rate;
    adc->conversion_timeout_ms = config->conversion_timeout_ms;

    uint16_t config_register;
    err = ads1115_read_register(adc, ADS1115_REG_CONFIG, &config_register);
    if (err != ESP_OK) {
        i2c_master_bus_rm_device(adc->device);
        memset(adc, 0, sizeof(*adc));
        return err;
    }

    adc->initialized = true;
    return ESP_OK;
}

esp_err_t ads1115_deinit(ads1115_t *adc)
{
    if (adc == NULL || !adc->initialized || adc->device == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = i2c_master_bus_rm_device(adc->device);
    if (err == ESP_OK) {
        memset(adc, 0, sizeof(*adc));
    }
    return err;
}

esp_err_t ads1115_read_single_ended(ads1115_t *adc,
                                    ads1115_channel_t channel,
                                    int16_t *raw_code,
                                    double *voltage)
{
    if (adc == NULL || !adc->initialized || raw_code == NULL ||
        channel < ADS1115_CHANNEL_AIN0 || channel > ADS1115_CHANNEL_AIN3) {
        return ESP_ERR_INVALID_ARG;
    }

    const uint16_t config = ADS1115_CONFIG_OS_START |
                            (ADS1115_CONFIG_MUX_AIN0_GND + ((uint16_t)channel << 12)) |
                            ((uint16_t)adc->full_scale << 9) |
                            ADS1115_CONFIG_MODE_SINGLE_SHOT |
                            ((uint16_t)adc->data_rate << 5) |
                            ADS1115_CONFIG_COMPARATOR_DISABLED;
    esp_err_t err = ads1115_write_register(adc, ADS1115_REG_CONFIG, config);
    if (err != ESP_OK) {
        return err;
    }

    const int64_t deadline_us = esp_timer_get_time() +
                                (int64_t)adc->conversion_timeout_ms * 1000;
    uint16_t current_config;
    do {
        err = ads1115_read_register(adc, ADS1115_REG_CONFIG, &current_config);
        if (err != ESP_OK) {
            return err;
        }
        if ((current_config & ADS1115_CONFIG_OS_READY) != 0U) {
            break;
        }
        vTaskDelay(1);
    } while (esp_timer_get_time() < deadline_us);

    if ((current_config & ADS1115_CONFIG_OS_READY) == 0U) {
        return ESP_ERR_TIMEOUT;
    }

    uint16_t conversion;
    err = ads1115_read_register(adc, ADS1115_REG_CONVERSION, &conversion);
    if (err != ESP_OK) {
        return err;
    }

    *raw_code = (int16_t)conversion;
    if (voltage != NULL) {
        *voltage = ads1115_raw_to_voltage(adc->full_scale, *raw_code);
    }
    return ESP_OK;
}
