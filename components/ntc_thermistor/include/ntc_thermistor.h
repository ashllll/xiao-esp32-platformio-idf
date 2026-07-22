#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double nominal_resistance_ohm;
    double beta_kelvin;
    double nominal_temperature_c;
    double fixed_resistor_ohm;
} ntc_beta_config_t;

/**
 * Calculate the NTC resistance for this divider:
 * supply -> fixed resistor -> ADC node -> NTC -> GND.
 */
bool ntc_low_side_resistance(double supply_voltage,
                             double node_voltage,
                             double fixed_resistor_ohm,
                             double *resistance_ohm);

/** Convert NTC resistance to degrees Celsius with the one-parameter Beta model. */
bool ntc_beta_temperature(double resistance_ohm,
                          const ntc_beta_config_t *config,
                          double *temperature_c);

/** Calculate resistance and temperature from a low-side NTC divider. */
bool ntc_low_side_temperature(double supply_voltage,
                              double node_voltage,
                              const ntc_beta_config_t *config,
                              double *resistance_ohm,
                              double *temperature_c);

#ifdef __cplusplus
}
#endif
