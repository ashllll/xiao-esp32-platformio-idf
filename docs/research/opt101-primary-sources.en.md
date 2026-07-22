# OPT101 primary-source review

Review scope: Texas Instruments OPT101 and generic OPT101 modules. Review date: 2026-07-18.

## Confirmed facts

- OPT101 integrates a photodiode and transimpedance amplifier and outputs an analog voltage related to optical power.
- Its spectral response is not the same as the photopic human-eye curve. Voltage cannot be called lux without application-specific optical calibration.
- Output range, feedback, loading, supply, and bypass requirements come from the IC datasheet, while a breakout may add a regulator, divider, filtering, or unknown routing.
- The XIAO ADC input must never exceed its electrical limits; gain/range protection must be designed for the brightest expected condition.

## Documentation consequences

Retain raw ADC code and calibrated voltage, describe any divider/filter, and separate electrical calibration from lux calibration. State geometry, spectrum, reference meter, range, and uncertainty for any physical-unit conversion.

## First-party sources

- [Texas Instruments OPT101 product page and datasheet](https://www.ti.com/product/OPT101)
- [ESP-IDF 6.0.1 ADC calibration guide](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32/api-reference/peripherals/adc_calibration.html)

## Open hardware evidence

The datasheet does not prove the schematic of a generic module or the accuracy of the assembled optical system. Validate darkness offset, known light levels, saturation, repeatability, temperature, and radio-active noise on the final hardware.
