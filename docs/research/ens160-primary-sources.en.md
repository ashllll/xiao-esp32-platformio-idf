# ENS160 primary-source review

Review scope: ScioSense ENS160 device behavior and the DFRobot Fermion ENS160 module SEN0515. Review date: 2026-07-18.

## Confirmed facts

- ENS160 is a metal-oxide air-quality sensor with status/validity states and environmental-compensation inputs.
- Its outputs include AQI, TVOC, and an estimated CO₂-equivalent result. eCO₂ is derived from gas response and is not direct CO₂ concentration measurement by NDIR or photoacoustic sensing.
- Startup and operating-mode transitions require time; data must not be treated as valid until the documented status permits it.
- Module-level supply, address selection, regulator, and level behavior must be taken from the exact DFRobot board documentation/schematic, not only the sensor IC datasheet.

## Documentation consequences

The integration page must report validity status with every measurement, label eCO₂ explicitly as an estimate, document warm-up and compensation, and avoid safety/regulatory claims. An I²C ACK or plausible number is communication evidence only.

## First-party sources

- [ScioSense ENS160 product page](https://www.sciosense.com/ens160-digital-metal-oxide-multi-gas-sensor/)
- [DFRobot SEN0515 product wiki](https://wiki.dfrobot.com/SKU_SEN0515_ENS160_Air_Quality_Sensor)
- [DFRobot SEN0515 repository](https://github.com/DFRobot/DFRobot_ENS160)

## Open hardware evidence

No source above proves that a particular module revision is wired correctly or that the current firmware produces valid readings. Capture module photos/revision, supply, address, status, warm-up, compensation source, and reference comparison during hardware acceptance.
