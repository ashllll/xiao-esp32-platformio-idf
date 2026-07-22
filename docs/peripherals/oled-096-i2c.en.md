# 0.96-inch 128×64 I²C OLED

“0.96-inch 128×64 I²C OLED” describes a product category, not a unique controller. Common modules use SSD1306 or SH1106-family controllers, with different memory layout and commands. Address labels such as `0x3C` are common but must be scanned and confirmed on the actual board.

## Before selecting a driver

1. Identify the controller from the seller documentation, markings, schematic, or known command behavior.
2. Confirm supply and I²C pull-up voltage.
3. Record resolution, visible offset, address, and reset wiring.
4. Choose an ESP-IDF-compatible driver for that controller and pin its version.

## Buffer and update policy

A 128×64 monochrome framebuffer needs 1024 bytes before driver overhead. Define ownership when multiple tasks draw, bound refresh rate, and handle bus errors. Avoid full-screen updates when partial updates meet the product requirement.

## Acceptance

Verify orientation, all edges/pixels, contrast, restart behavior, and long-run updates on the exact module. A successful I²C scan cannot identify the controller.

See the [primary-source review](../research/oled-128x64-primary-sources.md).
