# Hardware design, datasheet, and schematic policy

This page summarizes the current ADS1115 + NTC100K B3950 work and defines the project's ongoing component-selection, datasheet, and schematic rules.

## Current work summary

The project now includes:

- an ADS1115 single-shot component using the ESP-IDF 6.0.1 I²C master API;
- a low-side NTC divider and single-Beta conversion component;
- a shared three-board example configured for R25=100 kΩ, Beta=3950 K, and a 100 kΩ fixed resistor;
- AIN0 divider-node and AIN1 excitation measurements so 3.3 V is not treated as an exact constant;
- XIAO ESP32C3, ESP32S3, and ESP32C6 build coverage;
- host tests, bilingual wiring/equation/error-budget guidance, and a primary-source review.

Entry points:

- [ADS1115 + NTC100K B3950 guide](../peripherals/ads1115.md)
- [ADS1115 and NTC100K primary-source review](../research/ads1115-ntc100k-primary-sources-2026-07-22.md)
- Example: `examples/ads1115_ntc100k`
- Components: `components/ads1115` and `components/ntc_thermistor`

The evidence currently covers code, formula tests, target compilation, and documentation builds. No physical ADS1115 module, exact thermistor part, traceable thermometer, or three-board measurement campaign has established final accuracy or production readiness.

## Default manufacturer and source policy

When the user has not specified a component manufacturer:

1. Prefer a suitable Texas Instruments (TI) device.
2. Obtain its datasheet from the official `ti.com` source and record the full part number, document number, revision, issue/revision date, and retrieval date.
3. Use the datasheet's Recommended Application, Typical Application, or an official TI reference-design schematic as the baseline for the project's actual schematic.
4. Treat distributor pages, third-party modules, mirrors, and aggregate datasheet sites as discovery clues only.

When the user specifies a manufacturer or exact part, use that manufacturer's official documentation instead of forcing a TI substitution.

If TI has no suitable part or a project constraint requires another vendor, never invent a TI part or misattribute a datasheet. Explain the exception, obtain user confirmation before switching vendors, and retain the selected manufacturer's official source, version, and design boundaries.

## From reference circuit to project schematic

A typical application must not simply be copied without analysis. Preserve this mapping for every design:

| Item | Required record |
| --- | --- |
| Reference | TI part, datasheet number/revision, and application figure or section |
| Conditions | Input/output voltage, current, load, frequency, temperature, and startup |
| Calculations | Resistor, capacitor, inductor, compensation, sampling, protection, and ratings |
| Deviations | Every changed connection/value and its rationale |
| Protection/interfaces | ESD, overvoltage, reverse polarity, surge, current limiting, levels, and failure state |
| Layout | Decoupling, return paths, analog/switch nodes, thermal path, clearance, and grounding |
| Evidence | Calculations, simulation, ERC, DRC, bench tests, temperature rise, and fault tests |

ERC and DRC establish only their respective rule checks. They do not replace electrical analysis, rating checks, or hardware validation.

## ADS1115 + NTC100K cautions

- ADS1115 is an explicitly selected TI device and uses TI's official datasheet.
- `NTC100K B3950` specifies R25 and Beta characteristics, not a complete manufacturer part number. Beta endpoints, tolerances, R/T curves, self-heating, and packages differ.
- TI does not provide a generic discrete thermistor datasheet that can stand in for every NTC100K B3950, so the TI default must not be misapplied to the unknown NTC.
- Until an exact thermistor part is selected, the divider is an example connection, not an approved production schematic.
- The 100 kΩ/100 kΩ divider has about 50 kΩ Thevenin resistance at 25 °C. High-accuracy work needs a low-bias buffer, precision fixed resistor, exact R/T table or Steinhart–Hart coefficients, and two- or three-point assembled-system calibration.
- Sixteen-bit resolution is not sixteen-bit temperature accuracy. Treat approximately ±0.5 °C over the intended range as an optimization target, not a promise, until physical error-budget and calibration evidence exists.

## Acceptance checklist

- [ ] Manufacturer and full part number are fixed, or the exception is approved.
- [ ] Official datasheet URL, document number, revision, and retrieval date are recorded.
- [ ] The schematic traces to an official reference and every deviation is justified.
- [ ] Supply, interfaces, absolute limits, tolerance, drift, self-heating, and faults are checked.
- [ ] PCB implementation follows decoupling, return-path, layout, and thermal guidance.
- [ ] ERC, DRC, builds, and software tests pass.
- [ ] Target-board bench, temperature-point, and fault testing use real components.
- [ ] Documentation distinguishes source review, compilation, and hardware validation.

This policy is a project-level constraint from 2026-07-22 and is also recorded in the repository-root `AGENTS.md` for future development agents.
