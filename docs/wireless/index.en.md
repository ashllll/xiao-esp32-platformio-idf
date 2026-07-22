# Wireless connectivity

## Board capability

C3, S3, and C6 support 2.4 GHz Wi-Fi and Bluetooth LE. C6 additionally integrates IEEE 802.15.4 hardware for Thread/Zigbee-class protocols. A radio capability does not by itself provide a complete protocol stack, product role, commissioning flow, certification, or tested coexistence configuration.

## Wi-Fi

Document STA/AP mode, reconnect policy, power saving, offline behavior, TLS trust source, time synchronization, and provisioning. Keep SSIDs and credentials out of source control. Test loss of AP, invalid credentials, DHCP/DNS failure, certificate failure, and recovery.

## BLE

Define central/peripheral and client/server roles, UUIDs, properties, payload encoding, security, MTU, connection parameters, and advertising/reconnect behavior. Select the NimBLE or Bluedroid APIs/examples that match ESP-IDF 6.0.1 and test discovery, read/write, notify/indicate, disconnect, and re-advertising.

## Zigbee, Thread, and Matter

Use C6 for integrated IEEE 802.15.4. Pin compatible managed-component versions, partitions, role, channel, commissioning, factory reset, and credential lifecycle. Verify the exact Wi-Fi/BLE/802.15.4 coexistence combination against ESP-IDF 6.0.1 documentation and on hardware.

## Security baseline

- Do not commit private keys, tokens, production certificates, or Wi-Fi credentials.
- Validate peers and certificate chains; do not disable verification as a permanent fix.
- Bound retries and memory use for untrusted traffic.
- Define update, key rotation, factory reset, and lost-network behavior.

Official entry points: [ESP-IDF 6.0.1](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/) and [ESP32-C6 guide](https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32c6/).
