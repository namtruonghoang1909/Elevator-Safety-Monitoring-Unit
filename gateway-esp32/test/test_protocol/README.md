# Protocol Verification Test

This test module verifies the correctness of the shared protocol definitions on the ESP32 platform.

## Scope
- Verifies that all `protocol_packets.h` structures are exactly 8 bytes (essential for CAN frames).
- Verifies struct member alignment and offsets.
- Ensures cross-platform compatibility by strictly enforcing packing.

## How to Run
```bash
pio test -e esp32doit-devkit-v1 -f test_protocol
```
