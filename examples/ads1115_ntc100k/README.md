# ADS1115 + NTC 100K/B3950 example

This standalone PlatformIO + ESP-IDF example uses the repository's `ads1115`,
`ntc_thermistor`, and `xiao_board` components. AIN0 reads the divider node and
AIN1 reads the divider's 3.3 V supply.

Build without writing hardware:

```bash
pio run -d examples/ads1115_ntc100k -e xiao_esp32c3
pio run -d examples/ads1115_ntc100k -e xiao_esp32s3
pio run -d examples/ads1115_ntc100k -e xiao_esp32c6
```

See the bilingual ADS1115 peripheral page for wiring, accuracy limits,
parameter changes, and hardware acceptance. Flashing remains an explicit,
separate operation.
