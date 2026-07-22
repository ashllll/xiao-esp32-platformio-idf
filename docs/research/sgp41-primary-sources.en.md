# SGP41 primary-source review

Review scope: Sensirion SGP41 and the official Gas Index Algorithm. Review date: 2026-07-18.

## Confirmed facts

- SGP41 provides raw VOC and NOx signals over I²C.
- Sensirion's Gas Index Algorithm converts these signals to VOC and NOx indices when used with the required sampling/state behavior.
- The device does not directly output TVOC ppm, eCO₂, or CO₂.
- Temperature and humidity compensation uses defined encoded inputs; stale or incorrectly scaled values can bias the output.
- Generic GY-SGP41 breakout boards are not one controlled design. Regulator, level shifting, pull-ups, and safe supply/signals must be verified on the real module.

## Documentation consequences

Keep raw signals distinct from algorithm indices, preserve the required cadence and algorithm state, and label conditioning/startup periods. Do not convert index values into unsupported concentration units or safety claims.

## First-party sources

- [Sensirion SGP41 product page](https://sensirion.com/products/catalog/SGP41)
- [Sensirion gas-index algorithm repository](https://github.com/Sensirion/gas-index-algorithm)
- [Sensirion embedded SGP driver repository](https://github.com/Sensirion/embedded-sgp)

## Open hardware evidence

The official sources prove IC behavior, not the electrical design of an anonymous breakout or the current project's runtime result. Hardware acceptance must record the module schematic/revision, signal levels, cadence, conditioning state, and test exposure.
