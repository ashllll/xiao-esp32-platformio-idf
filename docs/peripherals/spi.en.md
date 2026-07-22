# SPI

The shared header symbols are D8/SCK, D9/MISO, and D10/MOSI. Assign a separate chip-select such as D3 only after checking board and expansion conflicts.

## Record per device

- SPI mode (CPOL/CPHA);
- maximum and tested clock rates;
- bit order and command/address phase;
- chip-select timing and polarity;
- full- or half-duplex behavior;
- supply, logic level, and peak current.

MISO may be omitted for write-only displays. Multiple devices can share SCK/MOSI/MISO only when inactive devices release MISO and each has an independent chip-select.

## ESP-IDF ownership

Initialize one bus owner, add device handles, serialize shared access, and free devices before the bus. DMA buffers must satisfy the active driver's allocation/alignment rules. Propagate timeouts rather than retrying forever.

## Acceptance

Start below the module's maximum rate, verify a device ID or deterministic transfer, then test the intended clock, cable, and concurrent-device configuration on hardware. A clean build does not validate signal integrity.

Official API: [ESP-IDF 6.0.1 SPI master](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32/api-reference/peripherals/spi_master.html).
