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
