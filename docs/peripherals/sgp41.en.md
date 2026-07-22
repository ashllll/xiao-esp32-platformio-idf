# SGP41 VOC/NOx sensor

SGP41 produces raw VOC and NOx signals. Sensirion's Gas Index Algorithm can convert these into VOC and NOx indices. The sensor does not directly output TVOC ppm, eCO₂, or CO₂.

## Module uncertainty

Generic “GY-SGP41” boards vary. Inspect the real board for regulator, level shifting, pull-ups, and voltage labels. Do not assume that a wide supply claim makes SDA/SCL 5 V safe.

## Integration

- Use 3.3 V-safe I²C wiring and the documented address/commands.
- Supply temperature/humidity compensation using the encoding and timing defined by Sensirion.
- Preserve algorithm state and sampling cadence according to the official algorithm documentation.
- Treat startup conditioning and interrupted sampling as explicit states.
- Report raw signals separately from calculated indices.

## Acceptance

First validate communication and self-test/status behavior. Then run the complete sensor and algorithm at the required cadence under controlled exposure. Trend response is not a calibrated concentration measurement or a safety alarm.

See the [primary-source review](../research/sgp41-primary-sources.md).
