# Project development workflow

This workflow keeps firmware, wiring, and documentation in one verifiable state.

## 1. Define the target

Record the exact board and variant, peripheral module revision, supply and logic voltages, bus, sample rate, latency, power target, wireless mode, credential flow, failure behavior, and acceptance signal.

## 2. Allocate resources

1. Select header pins from the [pinout](hardware/pinout.md).
2. Exclude strap pins, native USB, user LED, and expansion-board conflicts.
3. Record addresses, chip-selects, pull-ups, and maximum bus rates.
4. Check peak current, grounding, and back-power paths.

Do not start a driver without a wiring table.

## 3. Choose a driver source

Use ESP-IDF built-ins first, then maintained Espressif Component Registry packages, then vendor-maintained ESP-IDF components, and finally a local component. Record name, exact version, license, supported targets, examples, and review date. Arduino APIs do not work in this ESP-IDF project unless Arduino is deliberately added as a component.

## 4. Implement one vertical slice

```text
power and wiring -> bus init -> device probe -> one transaction -> error log -> docs
```

Reusable drivers belong under `components/<device>/`. Public APIs should return `esp_err_t`, keep state in an explicit handle/context, and expose failures to the application.

## 5. Validate in layers

| Layer | Evidence |
|---|---|
| `pio run -e <env>` | APIs, configuration, compilation, and linking |
| generated `sdkconfig.h` | effective Flash, PSRAM, and feature flags |
| serial probe/read | basic wiring, address, and driver behavior |
| long run and fault injection | recovery and resource lifecycle |
| C3/S3/C6 build matrix | shared code has no board-specific regression |
| `python3 scripts/validate_docs.py` | versions, links, anchors, bilingual parity, and navigation |
| `mkdocs build --strict` | both localized sites render without warnings |

## 6. Deliver and document

Use the [release guide](firmware/release.md) to package the application, bootloader, partition table, flash arguments, manifest, and checksums. Mark hardware as verified only after the exact revision passes on the named device.

For new hardware, create a page from `docs/peripherals/_template.md`, cite official sources, include a review date, and state untested assumptions explicitly.
