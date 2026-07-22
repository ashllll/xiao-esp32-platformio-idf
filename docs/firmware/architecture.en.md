# Firmware architecture

## Layout

```text
components/xiao_board/   board profile, runtime checks, and LED abstraction
include/xiao_pins.h      D0-D10 symbolic mapping
src/main.c               minimal application entry point
scripts/                 packaging, documentation, and hardware verification
tests/                   host-side regression tests
```

`app_main()` initializes the board abstraction, verifies the selected profile, prints `XIAO_RUNTIME_READY version=1`, and then emits heartbeats. Product tasks must start only after the runtime check succeeds.

## Board abstraction

The shared component exposes board name, expected chip/Flash/PSRAM properties, optional user-LED control, and runtime validation. It deliberately does not claim to identify the manufacturer or expansion-board variant.

Application code should use `XIAO_D*` and bus aliases instead of raw GPIO numbers. A reusable peripheral driver belongs under `components/<device>/` with a stable public header and direct CMake dependencies.

## Console policy

The application console uses USB Serial/JTAG so header D6/D7 remain available to a product UART. ROM output occurs before ESP-IDF takes control and may still use UART0. Do not burn eFuses merely to hide boot output.

## Error policy

Initialization functions return explicit errors. Drivers must not suppress bus failures or keep running with an invalid board profile. Logs should state expected and observed values without exposing credentials.

## Extension rules

- Keep ownership and lifetime in an explicit handle/context.
- Use `REQUIRES` for public component dependencies and `PRIV_REQUIRES` for implementation-only dependencies.
- Add Kconfig/defaults only for settings that must be reproducible.
- Document wiring, power, versions, failure modes, and hardware acceptance with the implementation.
