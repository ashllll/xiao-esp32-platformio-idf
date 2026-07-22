# SHT40 temperature and humidity sensor

SHT40 measures temperature and relative humidity and can provide environmental compensation for gas sensors.

## Address and module boundary

The commonly used SHT40 address is `0x44`; related product variants can use other addresses. Confirm the exact part and breakout board. Qwiic connections in this project are treated as 3.3 V logic.

## Integration

- Follow Sensirion command, conversion-delay, and CRC requirements.
- Check the CRC before converting or publishing a sample.
- Do not place the sensor next to a regulator, MCU, display, or heater if ambient measurements matter.
- Avoid condensation and respect the datasheet operating range and recovery guidance.
- Define retry limits and report stale compensation data.

## Acceptance

Compare against a suitable reference only after thermal equilibrium in a representative enclosure. Board self-heating and airflow can dominate the error even when the digital transaction is correct.

See the [primary-source review](../research/sht40-primary-sources.md).
