# 128×64 OLED primary-source review

Review scope: controller identification for generic 0.96-inch I²C OLED modules. Review date: 2026-07-18.

## Confirmed facts

- Size, resolution, color, and I²C interface do not uniquely identify the controller.
- SSD1306 and SH1106-family modules can require different commands, memory mapping, and visible-column offsets.
- `0x3C` is common but not guaranteed; an address ACK still cannot identify the controller.
- Module supply and pull-up voltage depend on the breakout design, not only the display-controller IC.

## Documentation consequences

Identify the controller and module schematic before selecting a driver. Record address, reset wiring, orientation, offsets, buffer ownership, and refresh policy. Keep controller-specific initialization out of a falsely generic “0.96 OLED” abstraction.

## First-party sources

- [Solomon Systech SSD1306 product page](https://www.solomon-systech.com/product/ssd1306/)
- The controller datasheet and module-vendor schematic supplied with the exact purchased unit

## Open hardware evidence

No generic listing proves which controller is fitted. Hardware acceptance should cover every display edge, orientation, contrast, restart, sustained refresh, and supply behavior on the actual module.
