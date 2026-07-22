# SHT40 primary-source review

Review scope: Sensirion SHT4x family and SHT40 integration. Review date: 2026-07-18.

## Confirmed facts

- SHT40 measures digital temperature and relative humidity.
- Commands have specified conversion delays and return data protected by CRC; software must verify CRC before publishing a sample.
- The common SHT40 address is `0x44`; related SHT4x variants can use other addresses, so the exact part remains important.
- Placement, self-heating, airflow, condensation, and thermal equilibrium affect system accuracy even when digital communication is correct.

## Documentation consequences

Record measurement mode/repeatability, delay, CRC policy, retry limit, placement, and stale-data behavior. When SHT40 feeds gas-sensor compensation, propagate freshness and validity instead of silently reusing old values.

## First-party sources

- [Sensirion SHT40 product page](https://sensirion.com/products/catalog/SHT40)
- [Sensirion embedded SHT repository](https://github.com/Sensirion/embedded-sht)

## Open hardware evidence

Datasheet specifications do not prove enclosure-level accuracy. Compare the assembled product with a suitable reference after thermal equilibrium, under representative airflow and radio/load conditions.
