# ESMU Code Implementation Plan

---

## 1. Display Service Implementation Plan (System State Registry)

### Architectural Strategy: The "Whiteboard" Pattern
The `display_service` is a **pure observer** of the system state. It reads telemetry from the **System Status Registry** and renders the corresponding UI.

### Implementation Steps
1. **Registry**: Implement `system_registry.h/c` in `components/system`.
2. **Assets**: Implement `display_font.h` (ASCII) and `display_icons.h`.
3. **Display Service**: Implement Layer 1 (Primitives) -> Layer 2 (UI Components) -> Layer 3 (Manager).
4. **Integration**: Hook existing services into the Registry and verify the data flow.

---

## 2. WiFi Provisioning System Plan (SoftAP + Web Server)

### Objective
Implement a user-friendly WiFi provisioning system that allows the ESMU to boot into Access Point (AP) mode, host a web portal for entering home WiFi credentials, and persist these credentials in NVS.

### Implementation Steps
1. **NVS Storage**: Implement `nvs_storage_init/save/load/clear`.
2. **Web Server**: Implement `esp_http_server` with a POST handler for `/config`.
3. **WiFi Manager**: Refactor `wifi_sta.c` to support both AP and STA modes.
4. **Connectivity Orchestration**: Handle the flow between Provisioning and Normal Operation.

---

## 3. Shared Node Communication Protocol

### Objective
Define a robust, cross-platform binary protocol for data exchange between ESP32 and STM32.

### File Structure (`shared/protocol/`)
1. **`protocol_types.h`**: 
   - Define `system_state_t` (INIT, CALIBRATING, RUNNING, ERROR).
   - Define `fault_code_t` (SHAKE, STOP, FALL, OVERTILT).
   - Define `balance_state_t` (LEVEL, TILT_LEFT/RIGHT/FORWARD/BACK).
2. **`protocol_packets.h`**: 
   - `ele_health_t`: `int16_t avg_tilt`, `int16_t max_tilt`, `uint8_t balance_state`, `uint8_t health_score`.
   - `ele_emergency_t`: `uint8_t fault_code`, `uint8_t severity`, `int16_t fault_value`.
   - `edge_heartbeat_t`: `uint8_t edge_health`, `uint32_t uptime_sec`.
   - **Constraint**: All structs MUST use `__attribute__((packed))` and be padded to exactly 8 bytes.
3. **`protocol.h`**:
   - Define CAN IDs: `CAN_ID_ELE_EMERGENCY (0x010)`, `CAN_ID_ELE_HEALTH (0x100)`, `CAN_ID_EDGE_HEALTH (0x200)`.

---

## 4. CAN Platform (ESP32 Gateway)

### Objective
Create a reusable, thread-safe TWAI abstraction layer similar to `i2c_platform`.

### Implementation Details (`can_platform`)
1. **`can_bus_init(const can_config_t *cfg)`**:
   - Map `baud_rate_kbps` to `twai_timing_config_t`.
   - Set `twai_general_config_t` with standard TX/RX pins.
   - Install driver with a 10-frame TX/RX queue.
2. **`can_bus_transmit(uint32_t id, const uint8_t *data, uint8_t len)`**:
   - Protect with a Mutex if multiple services transmit.
   - Format `twai_message_t` as Standard Frame, non-RTR.
3. **`can_bus_receive(uint32_t *id, uint8_t *data, uint8_t *len, uint32_t timeout_ms)`**:
   - Wrap `twai_receive` and extract raw fields for the protocol layer.

### Integration (CMake)
Update `gateway-esp32/CMakeLists.txt`:
```cmake
idf_component_register(
    ...
    INCLUDE_DIRS "include" "../../shared"
    ...
)
```

---

## 5. Verification & Testing
1. **Protocol Alignment**: Build a small C utility for both ESP32 and STM32 to print `sizeof()` each packet.
2. **Loopback Verification**: Use `CAN_MODE_LOOPBACK` on the Gateway to verify the `can_platform` can send and receive the defined protocol packets.
