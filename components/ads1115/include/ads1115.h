#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ADS1115_CHANNEL_AIN0 = 0,
    ADS1115_CHANNEL_AIN1,
    ADS1115_CHANNEL_AIN2,
    ADS1115_CHANNEL_AIN3,
} ads1115_channel_t;

typedef enum {
    ADS1115_FSR_6_144V = 0,
    ADS1115_FSR_4_096V,
    ADS1115_FSR_2_048V,
    ADS1115_FSR_1_024V,
    ADS1115_FSR_0_512V,
    ADS1115_FSR_0_256V,
} ads1115_fsr_t;

typedef enum {
    ADS1115_DATA_RATE_8_SPS = 0,
    ADS1115_DATA_RATE_16_SPS,
    ADS1115_DATA_RATE_32_SPS,
    ADS1115_DATA_RATE_64_SPS,
    ADS1115_DATA_RATE_128_SPS,
    ADS1115_DATA_RATE_250_SPS,
    ADS1115_DATA_RATE_475_SPS,
    ADS1115_DATA_RATE_860_SPS,
} ads1115_data_rate_t;

typedef struct {
    uint8_t i2c_address;
    uint32_t scl_speed_hz;
    ads1115_fsr_t full_scale;
    ads1115_data_rate_t data_rate;
    uint32_t conversion_timeout_ms;
} ads1115_config_t;

typedef struct {
    i2c_master_dev_handle_t device;
    ads1115_fsr_t full_scale;
    ads1115_data_rate_t data_rate;
    uint32_t conversion_timeout_ms;
    bool initialized;
} ads1115_t;

#define ADS1115_CONFIG_DEFAULT()             \
    {                                        \
        .i2c_address = 0x48,                 \
        .scl_speed_hz = 100000,              \
        .full_scale = ADS1115_FSR_4_096V,    \
        .data_rate = ADS1115_DATA_RATE_128_SPS, \
        .conversion_timeout_ms = 25,         \
    }

/** Add and probe one ADS1115 on an existing ESP-IDF I2C master bus. */
esp_err_t ads1115_init(ads1115_t *adc,
                       i2c_master_bus_handle_t bus,
                       const ads1115_config_t *config);

/** Remove the ADS1115 device handle. The caller still owns the I2C bus. */
esp_err_t ads1115_deinit(ads1115_t *adc);

/** Run one single-shot, single-ended conversion against GND. */
esp_err_t ads1115_read_single_ended(ads1115_t *adc,
                                    ads1115_channel_t channel,
                                    int16_t *raw_code,
                                    double *voltage);

/** Convert a signed ADS1115 code using the configured full-scale range. */
double ads1115_raw_to_voltage(ads1115_fsr_t full_scale, int16_t raw_code);

#ifdef __cplusplus
}
#endif
