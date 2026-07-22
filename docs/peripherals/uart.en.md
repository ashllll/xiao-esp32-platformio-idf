# UART

The project reserves XIAO D6/D7 as the product UART while the application console uses USB Serial/JTAG. ROM boot output may still appear on UART0 before ESP-IDF takes control.

## Wiring

Cross TX to RX, connect ground, and use 3.3 V logic. RS-232 and RS-485 electrical levels require transceivers; they cannot connect directly to XIAO GPIO. Record baud, data bits, parity, stop bits, flow control, and message framing.

## Software policy

Install the ESP-IDF UART driver with bounded buffers and timeouts. Define ownership if several tasks share the port. Treat framing, parity, overflow, and disconnect events explicitly, and never log secret payloads.

## Acceptance

Test loopback first, then the actual peer at its real cable length and rate. Verify fragmented frames, silence/timeouts, reconnect, and invalid input. A readable boot message is not proof that the product protocol is correct.

Official API: [ESP-IDF 6.0.1 UART](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32/api-reference/peripherals/uart.html).
