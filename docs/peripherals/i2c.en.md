# I²C

Default symbols are `XIAO_I2C_SDA` (D4) and `XIAO_I2C_SCL` (D5). The raw GPIO differs between C3, S3, and C6, so application code must use the symbols.

## Electrical rules

I²C lines are open-drain and require pull-ups to a safe logic rail, normally 3.3 V here. Several breakout boards may already contain pull-ups; excessive parallel pull-up strength can violate sink-current or edge requirements. Keep wiring short, share ground, and start at 100 kHz when module or cable characteristics are uncertain.

Do not infer signal safety from a module's advertised supply range. Confirm whether its SDA/SCL pull-ups connect to 3.3 V, 5 V, or an onboard level shifter.

## ESP-IDF flow

Use the ESP-IDF I²C master driver matching the pinned framework. Install a bus, add device handles with explicit address and clock rate, perform bounded transactions, and delete handles during teardown. Check every `esp_err_t`.

## Address scan limits

A scan is useful for wiring diagnosis, but an ACK identifies only an occupied address. It does not prove controller type, sensor health, calibration, or measurement validity. Avoid probing reserved addresses or issuing device-specific writes during a generic scan.

## Acceptance

- correct voltage and idle-high lines;
- expected address ACKs consistently;
- device identity/status register matches its datasheet when available;
- repeated reads survive disconnect/reconnect according to the documented policy;
- bus errors are logged and propagated.

Official API: [ESP-IDF 6.0.1 I²C master driver](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32/api-reference/peripherals/i2c.html).
