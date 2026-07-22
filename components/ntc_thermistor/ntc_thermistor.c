#include "ntc_thermistor.h"

#include <math.h>
#include <stddef.h>

#define KELVIN_OFFSET 273.15

static bool positive_finite(double value)
{
    return isfinite(value) && value > 0.0;
}

bool ntc_low_side_resistance(double supply_voltage,
                             double node_voltage,
                             double fixed_resistor_ohm,
                             double *resistance_ohm)
{
    if (resistance_ohm == NULL || !positive_finite(supply_voltage) ||
        !positive_finite(node_voltage) || node_voltage >= supply_voltage ||
        !positive_finite(fixed_resistor_ohm)) {
        return false;
    }

    const double resistance = fixed_resistor_ohm * node_voltage /
                              (supply_voltage - node_voltage);
    if (!positive_finite(resistance)) {
        return false;
    }
    *resistance_ohm = resistance;
    return true;
}

bool ntc_beta_temperature(double resistance_ohm,
                          const ntc_beta_config_t *config,
                          double *temperature_c)
{
    if (config == NULL || temperature_c == NULL ||
        !positive_finite(resistance_ohm) ||
        !positive_finite(config->nominal_resistance_ohm) ||
        !positive_finite(config->beta_kelvin) ||
        !isfinite(config->nominal_temperature_c) ||
        config->nominal_temperature_c <= -KELVIN_OFFSET) {
        return false;
    }

    const double nominal_kelvin = config->nominal_temperature_c + KELVIN_OFFSET;
    const double inverse_kelvin = (1.0 / nominal_kelvin) +
                                  log(resistance_ohm / config->nominal_resistance_ohm) /
                                      config->beta_kelvin;
    if (!positive_finite(inverse_kelvin)) {
        return false;
    }

    const double result = (1.0 / inverse_kelvin) - KELVIN_OFFSET;
    if (!isfinite(result)) {
        return false;
    }
    *temperature_c = result;
    return true;
}

bool ntc_low_side_temperature(double supply_voltage,
                              double node_voltage,
                              const ntc_beta_config_t *config,
                              double *resistance_ohm,
                              double *temperature_c)
{
    if (config == NULL || resistance_ohm == NULL || temperature_c == NULL) {
        return false;
    }
    return ntc_low_side_resistance(supply_voltage, node_voltage,
                                   config->fixed_resistor_ohm, resistance_ohm) &&
           ntc_beta_temperature(*resistance_ohm, config, temperature_c);
}
