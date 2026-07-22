#include <assert.h>
#include <math.h>

#include "ntc_thermistor.h"

static int close_enough(double actual, double expected, double tolerance)
{
    return fabs(actual - expected) <= tolerance;
}

int main(void)
{
    const ntc_beta_config_t config = {
        .nominal_resistance_ohm = 100000.0,
        .beta_kelvin = 3950.0,
        .nominal_temperature_c = 25.0,
        .fixed_resistor_ohm = 100000.0,
    };

    double resistance;
    double temperature;
    assert(ntc_low_side_temperature(3.3, 1.65, &config, &resistance, &temperature));
    assert(close_enough(resistance, 100000.0, 0.001));
    assert(close_enough(temperature, 25.0, 0.0001));

    const double target_kelvin = 10.0 + 273.15;
    const double nominal_kelvin = 25.0 + 273.15;
    const double resistance_at_10c = config.nominal_resistance_ohm *
                                     exp(config.beta_kelvin *
                                         (1.0 / target_kelvin - 1.0 / nominal_kelvin));
    const double node_at_10c = 3.3 * resistance_at_10c /
                               (config.fixed_resistor_ohm + resistance_at_10c);
    assert(ntc_low_side_temperature(3.3, node_at_10c, &config,
                                    &resistance, &temperature));
    assert(close_enough(temperature, 10.0, 0.0001));

    assert(!ntc_low_side_temperature(3.3, 0.0, &config, &resistance, &temperature));
    assert(!ntc_low_side_temperature(3.3, 3.3, &config, &resistance, &temperature));
    assert(!ntc_low_side_temperature(3.3, 3.4, &config, &resistance, &temperature));
    return 0;
}
