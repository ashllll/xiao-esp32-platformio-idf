# GPIO, ADC, and PWM

## GPIO

Use XIAO symbols, select input/output/pull/interrupt configuration deliberately, and check strap or expansion-board conflicts. GPIO is 3.3 V logic and cannot directly drive inductive or high-current loads.

Mechanical inputs need a defined idle level and debounce policy. Interrupt handlers should do minimal work and hand off processing to a task.

## ADC

Record the selected channel, attenuation, expected input range, source impedance, and calibration method. Never exceed the GPIO voltage limit. ADC codes are not automatically accurate volts, lux, or other physical units; use the ESP-IDF calibration facilities and an external reference appropriate to the application.

Average or filter only after defining bandwidth and latency. Verify whether radio activity or board circuitry affects the chosen ADC path on the exact target.

## PWM with LEDC

LEDC produces a logic waveform, not load power. Document timer/channel ownership, frequency, duty resolution, polarity, startup state, and driver stage. Frequency and resolution trade off against one another; verify the generated result under the pinned ESP-IDF version.

Official APIs: [ESP-IDF 6.0.1 ADC oneshot](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32/api-reference/peripherals/adc_oneshot.html) and [LEDC](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32/api-reference/peripherals/ledc.html).
