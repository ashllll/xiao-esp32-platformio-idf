# ADS1115 + NTC100K primary-source review

Review date: 2026-07-22

This note records the evidence used for the ADS1115 + NTC100K example on ESP-IDF 6.0.1 and the XIAO ESP32C3, ESP32S3, and ESP32C6. It establishes design boundaries; it is not evidence that an unknown breakout or thermistor has passed physical testing.

## Conclusions

- Power ADS1115 and its I²C pull-ups from 3.3 V. Its operating supply range is 2.0–5.5 V, but every analog input must remain between GND and VDD in normal use.
- Use address `0x48` with ADDR tied to ground. The example uses single-shot AIN0-to-GND, ±4.096 V PGA, 128 SPS, comparator disabled, and polls the OS bit. The AIN0 Config word is `0xC383`.
- Use a 100 kΩ fixed resistor above the divider node and a 100 kΩ NTC below it. At 25 °C the ideal node is about half the excitation voltage.
- “NTC100K” normally specifies only R25. It does not establish Beta, Beta endpoints, tolerance, package, R/T curve, or dissipation factor. The example's 3950 K value is user-selected configuration.
- Read the actual excitation on AIN1. ADS1115 uses an internal reference, so assuming an exact 3.300 V excitation would directly add rail error to the calculated resistance.
- The Beta equation is an approximation. Use the exact part's R/T table or Steinhart–Hart coefficients and system calibration for tighter accuracy.

## ADS1115 constraints

TI's ADS1115 datasheet specifies 16-bit two's-complement results, full-scale settings from ±6.144 V to ±0.256 V, and data rates from 8 to 860 SPS. At ±4.096 V, one code is 125 µV. A selected full-scale range is a transfer-function scale, not permission to drive an input beyond the supply rails.

ADS1115 is a switched-capacitor delta-sigma ADC and does not present infinite input impedance. TI lists typical common-mode and differential input impedances of 6 MΩ and 15 MΩ at ±4.096 V. A 100 kΩ/100 kΩ divider has 50 kΩ Thevenin resistance at 25 °C, so input loading belongs in the error budget. A low-bias buffer or assembled-system calibration may be required.

The single-shot sequence is:

1. Write `0x01, 0xC3, 0x83` to start AIN0.
2. Read Config with a repeated START until OS bit 15 becomes one, with a bounded timeout.
3. Read two bytes from Conversion register `0x00` and interpret them as signed `int16_t`.
4. Multiply the code by 125 µV for the selected range.

ESP-IDF 6.0.1 provides this flow through `driver/i2c_master.h`, `i2c_new_master_bus()`, `i2c_master_bus_add_device()`, and `i2c_master_transmit_receive()`.

## Wiring across XIAO targets

The project consistently uses the XIAO D4/D5 aliases:

| Signal | ESP32C3 | ESP32S3 | ESP32C6 |
| --- | ---: | ---: | ---: |
| SDA / D4 | GPIO6 | GPIO5 | GPIO22 |
| SCL / D5 | GPIO7 | GPIO6 | GPIO23 |

Use `XIAO_I2C_SDA` and `XIAO_I2C_SCL` instead of raw GPIO numbers. Place 0.1 µF near ADS1115 VDD and make sure any breakout pull-ups terminate at 3.3 V.

```text
XIAO 3V3 ── Rfixed 100 kΩ ──+── ADS1115 AIN0
                              |
                         NTC 100 kΩ
                              |
                             GND

XIAO 3V3 ─────────────────────── ADS1115 AIN1
```

## Thermistor model

For the low-side NTC topology:

```text
Rntc = Rfixed × Vnode / (Vexc - Vnode)
T(K) = 1 / (1 / T0(K) + ln(Rntc / R0) / Beta)
```

The model requires positive finite resistances and Beta, Kelvin temperatures, and `0 < Vnode < Vexc`. Near either rail, resistance inversion becomes ill-conditioned and the application should report a likely open, short, or saturation fault.

A concrete TDK 100 kΩ part, `B57541G1104F000`, specifies B25/85=4072 K and B25/100=4092 K. This is useful counter-evidence to the assumption that every 100 kΩ thermistor is B3950; its constants must not replace those of the user's B3950 part.

At 3.3 V with two 100 kΩ elements, ideal current is 16.5 µA and NTC power at 25 °C is about 27.2 µW. Actual self-heating depends on the exact thermistor, PCB, enclosure, airflow, and contact medium.

## Error budget and acceptance

Include R25/Beta and model tolerances, fixed-resistor tolerance and drift, ADS1115 gain/offset/noise/loading, actual excitation, leakage and contamination, self-heating, thermal gradients, and response time. Validate with precision substitution resistors and at least two traceable temperature points. Record raw code, Vnode, Vexc, resistor values, part/lot, model, and calibration identity.

Compilation and an I²C ACK do not demonstrate temperature accuracy. The physical module identity, pull-up circuit, thermistor constants, actual 3.3 V rail, and final calibration remain hardware acceptance items.

## Primary sources

1. [Texas Instruments ADS1113/ADS1114/ADS1115 datasheet, SBAS444E](https://www.ti.com/lit/ds/symlink/ads1115.pdf)
2. [ESP-IDF 6.0.1 I²C programming guide](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32/api-reference/peripherals/i2c.html)
3. Seeed Studio pin maps for [XIAO ESP32C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#pin-map), [XIAO ESP32S3](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#hardware-overview), and [XIAO ESP32C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#pin-map)
4. [TDK Electronics B57541G1 datasheet](https://www.tdk-electronics.tdk.com/inf/50/db/ntc/NTC_Glass_enc_sensors_G1541_coated.pdf)
5. [Vishay Selecting NTC Thermistors](https://www.vishay.com/docs/33001/seltherm.pdf)

All sources were retrieved on 2026-07-22.
