# Power, USB, and electrical safety

## Voltage limits

ESP32 GPIO uses 3.3 V logic and is not 5 V tolerant. Never connect `5V/VBUS` directly to a GPIO. A module advertised as “5 V powered” is not necessarily 3.3 V-safe on its signal pins; inspect its schematic or datasheet. External supplies must share signal ground, and parallel supplies require a back-feed analysis.

## Current budget

Design for peak, not average, current. Wi-Fi transmission, camera activity, SD writes, display backlights, heaters, and motors can create short peaks. Record source, peak load, decoupling, cable drop, and brownout behavior. GPIO must not directly drive motors, relays, heaters, or high-current LEDs; use a suitable driver and protection.

## USB

If no serial device appears, first rule out a charge-only cable. Compare `pio device list` before and after connection and close programs that already own the port. On S3, GPIO19/GPIO20 normally implement native USB D-/D+; do not repurpose them when USB is required.

## Batteries

Seeed documents a 3.7 V lithium-battery connection for the XIAO ESP32C3. Confirm polarity before soldering. Charging, protection, and fuel-gauge behavior for other boards or expansion bases must be checked against their own documentation and must not be inferred from C3.

## Pre-power checklist

- Supply and signal voltages match the exact module.
- Grounds are connected and there is no unintended back-power path.
- External circuits do not force boot straps to an invalid level.
- Peak current fits the source, regulator, connector, and cable.
- Inductive or high-current loads have proper drivers and protection.
- Initial power-up is observable and preferably current-limited.

Last reviewed: 2026-07-22.
