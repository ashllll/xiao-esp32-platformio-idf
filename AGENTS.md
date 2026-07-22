# Project hardware-source and schematic rules

- If a component manufacturer is not specified, default to an appropriate Texas Instruments (TI) device and obtain its datasheet from the official `ti.com` source.
- Use the TI datasheet's Recommended Application, Typical Application, or official reference-design schematic as the baseline for the project's actual schematic.
- Adapt the reference circuit only from verified project requirements. Record changes to values, protection, power, grounding, filtering, thermal design, interfaces, and layout constraints.
- A copied reference schematic is not proof of electrical correctness. Check operating and absolute-maximum limits, tolerances, temperature drift, source/load conditions, startup and fault behavior; then complete ERC, DRC, and hardware validation.
- If TI has no suitable part, do not invent or misattribute a TI datasheet. Explain the exception and obtain user confirmation before selecting another manufacturer's official documentation.
- If the user specifies a manufacturer or exact part, use that manufacturer's official datasheet instead of the TI default.
- Distributor pages, third-party modules, mirrors, and aggregate datasheet sites are discovery clues only; they do not replace the original manufacturer's documentation.

For the current ADS1115 + NTC100K work, ADS1115 is a specified TI device. `NTC100K B3950` describes electrical characteristics, not a complete manufacturer part number; do not claim production accuracy or draw a production schematic until the exact thermistor part is selected and verified.
