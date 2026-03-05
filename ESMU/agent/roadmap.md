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
  - [ ] Implementation: High-pass filtering and gyro-zeroing.
  - [ ] Verification: Signal integrity and drift tests.
- **Display Service**
  - [ ] Implementation: UI State management and icons.
  - [ ] Verification: UI transition and layout tests.
- **Fault Detector**
  - [ ] Implementation: Shake and emergency stop logic.
  - [ ] Verification: Simulated fault detection tests.

---

## IV. System Layer
*Global orchestration and state management.*

- **System Controller**
  - [ ] Implementation: Central Finite State Machine (FSM).
  - [ ] Verification: System-wide integration and fail-safe tests.

---
*Last Updated: March 2026*
