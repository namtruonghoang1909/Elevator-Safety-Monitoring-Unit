# ESMU Project Roadmap

This file tracks the high-level progress of the Elevator Safety Monitoring Unit (ESMU).

---

## I. Drivers & Platform Layer
*Low-level hardware abstractions and chip-specific drivers.*

- **Hardware Abstraction Layer (HAL) - Gateway**
  - [x] Refactor from BSP to HAL naming (`hal_<module>_<action>`).
  - [x] I2C Platform: Thread-safe master abstraction.
  - [x] SPI Platform: DMA support for ST7789.
  - [x] CAN Platform: TWAI thread-safe driver.
  - [x] UART Platform: Background RX with callbacks.
  - [x] PWM Platform: Backlight control.
- **MPU6050**
  - [x] Implementation: Raw and scaled data processing (STM32 Ported).
  - [x] Verification: Interactive Unity tests (Gravity/FSR/Recovery). [COMPLETE]
- **SSD1306**
  - [x] Implementation: Framebuffer and primitive drawing.
  - [x] Verification: Interactive visual suite. [COMPLETE]
- **Native FreeRTOS (STM32)**
  - [x] Implementation: Direct migration from CMSIS-RTOS to Native API.
  - [x] Verification: System stability and resource allocation. [COMPLETE]

---

## II. Connectivity Layer
*Networking and cloud communication stack.*

- **Wifi Station**
  - [x] Implementation: Station mode with auto-reconnect.
  - [x] Verification: Connection stability tests.
- **MQTT Manager**
  - [x] Implementation: Async pub/sub wrapper.
  - [x] Verification: Broker communication tests.
- **Connectivity Manager**
  - [x] Implementation: Orchestration of WiFi and MQTT.
  - [x] Verification: End-to-end telemetry tests.
- **SIM A7680C Driver**
  - [x] Implementation: Generic UART HAL Integration, Line Buffering.
  - [x] Implementation: Signal strength parsing (CSQ).
  - [x] Implementation: Registration & Operator parsing (CREG, COPS).
  - [x] Verification: Network status monitoring (via Cellular Service).

---

## III. Service Layer
*High-level logic and data processing services.*

- **Motion Monitor (Professional)**
  - [x] Implementation: 100Hz sensing, EMA filtering, 3-axis calibration.
  - [x] Maintenance: Gyro-based vibration analysis (LOW/WARN/CRIT).
  - [x] Safety: Free Fall (<0.35g) and Sudden Impact (>1.6g).
  - [x] Verification: Signal integrity and real-world ride quality. [COMPLETE]
- **Cellular Service**
  - [x] Implementation: Background FSM (Idle, Init, Search, Ready, Recover).
  - [x] Integration: System Registry updates for UI/Telemetry.
  - [ ] Verification: Real-world network registration (Pending HW Power Fix).
- **Display Service**
  - [x] SSD1306 Driver (I2C) - [DEPRECATED/STABLE].
  - [x] Edge Logger (Diagnostic OLED Mode) - **ACTIVE**.
  - [x] ST7789 Driver (SPI) - **COMPLETED**.
  - [x] SPI HAL Component (DMA Support) - **COMPLETED**.
  - [x] PWM HAL Component (Backlight) - **COMPLETED**.
  - [ ] UI Adaptation for Color/Higher Resolution.
- **Fault Detector**
  - [ ] Implementation: Shake and emergency stop logic.
  - [ ] Verification: Simulated fault detection tests.

---

## IV. Communication & Distributed System
*Inter-processor communication and shared protocols.*

- **Shared Protocol**
  - [x] Implementation: `protocol_types.h`, `protocol_packets.h`, `esmu_protocol.h`.
  - [x] Verification: Struct alignment and size checks via Unity on ESP32. [COMPLETE]
- **CAN Platform (ESP32 Gateway)**
  - [x] Implementation: Thread-safe TWAI abstraction layer.
  - [x] Verification: Internal loopback testing (GPIO Matrix). [COMPLETE]

---

## V. System Layer
*Global orchestration and state management.*

- **System Controller**
  - [ ] Implementation: Central Finite State Machine (FSM).
  - [ ] Verification: System-wide integration and fail-safe tests.

---
*Last Updated: April 2026*
