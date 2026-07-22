# Pinout

Use the XIAO header labels (`D0`–`D10`) in wiring notes and the symbols from `include/xiao_pins.h` in firmware.

| XIAO | ESP32C3 | ESP32S3 | ESP32C6 | Typical role |
|---|---:|---:|---:|---|
| D0 | GPIO2 | GPIO1 | GPIO0 | ADC / GPIO |
| D1 | GPIO3 | GPIO2 | GPIO1 | ADC / GPIO |
| D2 | GPIO4 | GPIO3 | GPIO2 | ADC / GPIO |
| D3 | GPIO5 | GPIO4 | GPIO21 | GPIO / chip select |
| D4 | GPIO6 | GPIO5 | GPIO22 | I²C SDA |
| D5 | GPIO7 | GPIO6 | GPIO23 | I²C SCL |
| D6 | GPIO21 | GPIO43 | GPIO16 | UART TX |
| D7 | GPIO20 | GPIO44 | GPIO17 | UART RX |
| D8 | GPIO8 | GPIO7 | GPIO19 | SPI SCK |
| D9 | GPIO9 | GPIO8 | GPIO20 | SPI MISO |
| D10 | GPIO10 | GPIO9 | GPIO18 | SPI MOSI |

Convenience symbols include `XIAO_I2C_SDA`, `XIAO_I2C_SCL`, `XIAO_UART_TX`, `XIAO_UART_RX`, and the three `XIAO_SPI_*` signals.

## Conflicts and cautions

- C3 GPIO2, GPIO8, and GPIO9 are strap-related; attached circuits must not force an invalid boot state.
- C3 D6 can carry ROM startup UART output even though the application console uses USB Serial/JTAG.
- S3 GPIO19/GPIO20 normally serve native USB and are not header D6/D7.
- S3 Sense microSD uses CS=GPIO3, SCK=GPIO7, MISO=GPIO8, and MOSI=GPIO9, occupying D2/D8/D9/D10. GPIO21 remains the user LED.
- C6 user LED is GPIO15. This project maps the S3 user LED to GPIO21. The C3 profile intentionally exposes no equivalent user LED.
- All signals are 3.3 V logic and are not 5 V tolerant.

Confirm the exact board revision and official pinout before assigning antenna control, camera, microSD, or expansion-board signals.

Last reviewed against Seeed board documentation: 2026-07-22.
