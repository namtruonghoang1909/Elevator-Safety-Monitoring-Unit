# Current Task: Distributed System Integration & CAN Protocol

## Goal
Establish communication between ESP32 (Gateway) and STM32 (Edge) and move safety-critical logic to the Edge.

## Completed
- [x] Implement enhanced monitoring view (tilting elevator).
- [x] Rename internal display and motion monitor files for clarity.
- [x] Add explicit `esp_err_t` logging across all services.
- [x] **WiFi Provisioning System**:
    - [x] NVS storage for credentials.
    - [x] Web Server with URL decoding (fixed space issue).
    - [x] 5-second button hold trigger (GPIO 15).
    - [x] Status LED blinking feedback (GPIO 4).
    - [x] UI "CONFIG MODE" display view.

## Pending (Next Phase)
- [x] **CAN Platform Verification**: 
    - [x] Fix loopback mode bug in `can_platform.c`.
    - [x] Modify `main.c` to run a loopback test (transmit and receive).
- [ ] **Protocol Definition**: Create `shared/protocol/esmu_protocol.h` for CAN/UART data structures.
- [ ] **STM32 Porting**: Move MPU6050 driver and `motion_monitor` logic to `edge-stm32`.
- [ ] **Fault Detector**: Implement the core anomaly detection logic on the STM32.
- [ ] **CAN Driver**: Implement CAN communication on both platforms.
- [ ] **Gateway Update**: Modify ESP32 to visualize data received via CAN instead of local sensors.
