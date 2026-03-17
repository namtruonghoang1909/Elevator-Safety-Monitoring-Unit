# ESMU Code Implementation Plan

---

## 1. Display Service Implementation Plan (System State Registry)
[STATUS: COMPLETE]
- **Registry**: Thread-safe "Whiteboard" pattern in `system_registry.h/c`.
- **Assets**: UI Components and Icons for SSD1306.
- **Integration**: Services update the Registry; the UI refreshes at 15Hz.

---

## 2. WiFi Provisioning System Plan (SoftAP + Web Server)
[STATUS: COMPLETE]
- **NVS**: Persistence for SSID/Password.
- **Web Server**: URL-decoded form handling for credential entry.
- **Orchestration**: Seamless transition from Config Mode to Monitoring Mode.

---

## 3. Shared Node Communication Protocol
[STATUS: REFINED]
- **Protocol Header**: `shared/protocol/esmu_protocol.h` (Consolidated).
- **Enforcement**: 8-byte packed structures with explicit `uint8_t` for enums.
- **CAN IDs**:
  - `CAN_ID_ELE_EMERGENCY (0x010)`
  - `CAN_ID_ELE_HEALTH (0x100)`
  - `CAN_ID_EDGE_HEALTH (0x200)`
- **Verification**: `static_assert` checks in ESP32 `main.c` to guarantee 8-byte packing.

---

## 4. CAN Platform (ESP32 Gateway)
[STATUS: COMPLETE & VERIFIED]
- **Abstraction**: `can_platform.h/c` provides a thread-safe wrapper for TWAI.
- **Verification**: Hardware-less verification via GPIO Matrix loopback (TX -> RX).

---

## 5. Gateway Motion Proxy (ESP32)
[STATUS: IN PROGRESS]

### Objective
Transition the ESP32 from active sensing to a passive "Motion Proxy" that mirrors the STM32's state.

### Implementation Steps
1. **Refactor `motion_monitor`**: Rename or refactor to `motion_proxy`.
2. **CAN RX Service**: Implement a dedicated task that blocks on `can_receive`.
3. **Registry Bridge**:
   - `ele_health_packet_t` -> `system_registry_update_from_protocol_health()`.
   - `edge_heartbeat_packet_t` -> `system_registry_update_from_protocol_heartbeat()`.
4. **Watchdog**: Implement a timeout check (3s) to detect STM32 disconnects.

---

## 6. Edge Node (STM32) Implementation
[STATUS: NEXT PHASE]

### Objective
Implement the active safety monitoring and sensor processing on the STM32.

### Implementation Steps
1. **CAN Driver**: Implement a lightweight CAN manager matching the `can_platform` API.
2. **MPU6050 Driver**: Port the ESP32 I2C driver to STM32 HAL.
3. **Motion Logic**: Port EMA filters and tilt calculation to the STM32.
4. **Fault Detector**: Implement real-time anomaly detection (Shake, Free Fall).
5. **Protocol Task**: Transmit health packets at 100ms and heartbeat at 1000ms.

---

## 7. Verification & Final Integration
1. **End-to-End Test**: Wire ESP32 and STM32 via CAN transceivers.
2. **Latency Check**: Verify the OLED "Tilting" animation remains smooth (<150ms total latency).
3. **Fail-Safe Test**: Disconnect the CAN cable and verify the ESP32 UI shows "COMM LOSS".
