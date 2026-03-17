# ESMU Project Roadmap

This file tracks the high-level progress of the Elevator Safety Monitoring Unit (ESMU).

---

## I. Drivers & Platform Layer
*Low-level hardware abstractions and chip-specific drivers.*

- **I2C Platform**
  - [x] Implementation: Thread-safe master abstraction.
  - [x] Verification: Unit tests for bus management.
- **MPU6050**
  - [x] Implementation: Raw and scaled data processing.
  - [x] Verification: Interactive Unity tests (Gravity/FSR/Recovery).
- **SSD1306**
  - [x] Implementation: Framebuffer and primitive drawing.
  - [x] Verification: Interactive visual suite. [COMPLETE]

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

---

## III. Service Layer
*High-level logic and data processing services.*

- **Motion Monitor**
  - [x] Implementation: EMA filtering, gravity compensation, and FSM.
  - [x] Verification: Signal integrity and drift tests via Unity. [COMPLETE]
- **Display Service**
  - [x] Implementation: Layered UI, elevator visualization, and status icons.
  - [x] Verification: Visual UI testing. [COMPLETE]
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
*Last Updated: March 2026*
