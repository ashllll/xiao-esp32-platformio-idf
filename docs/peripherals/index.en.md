# Peripheral integration

This section separates bus fundamentals from module-specific claims. A product name or breakout-board color is not enough to identify its controller, regulator, level shifter, pull-ups, or address.

## Before connecting anything

1. Identify the exact board and module revision.
2. Confirm supply voltage, signal levels, peak current, and common ground.
3. Allocate symbolic XIAO pins and check strap, USB, LED, and expansion conflicts.
4. Record bus speed, address or chip-select, pull-ups, and cable limits.
5. Check a primary datasheet or official schematic.
6. Define a minimal hardware acceptance procedure.

## Guides

- [I²C](i2c.md), [SPI](spi.md), [UART](uart.md), and [GPIO/ADC/PWM](gpio-adc-pwm.md)
- [ENS160](ens160.md), [SGP41](sgp41.md), and [SHT40](sht40.md)
- [0.96-inch I²C OLED](oled-096-i2c.md)
- [OPT101](opt101.md) and [ADS1115](ads1115.md)

Use `docs/peripherals/_template.md` for a new page. Record the driver/component version, framework boundary, wiring, power, error policy, and source review date.

## Evidence boundary

Compilation does not prove a device is present. An I²C ACK does not prove calibrated measurements. Trend sensors and estimated air-quality outputs must not be presented as certified safety or regulatory measurements.
