# ESMU Unified Architecture & Protocol Design

## Objective
Establish a symmetrical, layered architecture across the ESP32 Gateway and STM32 Edge nodes, using a shared CAN protocol for communication.

## 1. Unified Layered Architecture (Mirror Design)
Both nodes now follow the same directory and naming convention:
- **Application (`App/` or `src/`)**: High-level orchestration.
- **Services (`services/`)**: Business logic (Telemetry, Fault Detection).
- **Drivers (`drivers/`)**: Hardware-specific device drivers (SSD1306, MPU6050).
- **BSP (`bsp/`)**: [Refactored from "platform"] Hardware Abstraction Layer (HAL wrappers for CAN, I2C, GPIO).

## 2. Shared Protocol Definition
- **CAN Message IDs**: `0x010` (Emergency), `0x100` (Health), `0x200` (Heartbeat).
- **Data Structures**: Defined in `shared/can_protocol/` for cross-platform binary compatibility.

## 3. Module Responsibilities (STM32 Edge)
- **`fault_detector`**: Processes raw IMU data to detect Shakes, Free Fall, and Emergency Stops.
- **`can_service`**: Encodes sensor/state data into ESMU protocol packets for transmission via `bsp_can`.
- **`mpu6050_driver`**: Ported from ESP32, optimized for STM32 HAL I2C.

## 4. Concurrency Model (FreeRTOS)
- **`MotionTask` (High Priority, 10ms)**: IMU sampling -> Fault Detection -> CAN Health update (100ms).
- **`SystemTask` (Low Priority, 1000ms)**: Heartbeat transmission -> Status LED blink.

---

## 5. Remote Fault Acknowledgment (New Idea)
**Objective**: Allow remote users/operators to acknowledge and clear latched faults from the CoreIoT dashboard.

**Flow**:
1.  **Fault Detected**: Edge node detects fault -> CAN Emergency packet -> Gateway (ESP32).
2.  **Publish**: Gateway publishes MQTT message with `ele_fault_code > 0` and `ele_fault_msg`.
3.  **User Action**: User presses "Acknowledge" button on CoreIoT dashboard.
4.  **Signal Back**: CoreIoT sends an MQTT message (e.g., to `esmu/cmd/ack`) to the ESP32 Gateway.
5.  **Local Clear**: 
    -   Gateway receives ACK.
    -   Gateway clears its local `system_registry` fault state (`fault_active = false`).
    -   Gateway resets `current_state` to `SYSTEM_STATE_MONITORING`.
6.  **Confirm Clear**: Gateway publishes a confirmation MQTT message with `ele_fault_code = 0` to clear the dashboard status.

**Implementation Strategy**:
-   **MQTT Subscription**: Gateway needs to subscribe to a command topic.
-   **Registry Update**: Add a function to `system_registry` to clear faults.
-   **CAN Command (Optional)**: If the Edge node also latches, Gateway may need to send a CAN command back to the Edge node to reset its state.
