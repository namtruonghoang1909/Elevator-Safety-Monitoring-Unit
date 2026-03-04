# ESMU Project Todo Management

This file tracks the overall project progress and the specific, approved steps for current task execution.

---

## I. GLOBAL PROJECT ROADMAP
*Status of high-level project milestones.*

### 1. Drivers & Platform Layer
- [x] **I2C Platform**: Multi-bus, thread-safe abstraction.
- [x] **MPU6050 Driver**: Basic movement/gyro sensing (Verified in main).
- [x] **SSD1306 Driver**: OLED display primitives (Verified in main).
- [x] **Unity Driver Tests**: Formalize driver verification in the `test/` folder (MPU6050 Complete).

### 2. Connectivity Layer
- [x] **Wifi Station**: Stable connection with auto-reconnect.
- [x] **MQTT Manager**: Wrapper for telemetry and alerts.
- [x] **Connectivity Manager**: High-level orchestrator.
- [x] **Connectivity Tests**: Fully verified via Unity.

### 3. Service Layer (Abstraction)
- [ ] **Display Service**: High-level UI management (status icons, messages).
- [ ] **Motion Monitor**: Filters and processes raw MPU6050 data.
- [ ] **Fault Detector**: Logic for detecting shakes and emergency stops.

### 4. System Layer
- [ ] **System Controller**: Central FSM to coordinate all services.

---

## II. ACTIVE TASK EXECUTION
*Detailed sub-tasks for the current directive. **DO NOT START until approved by user.***

(No active tasks)

---
**Note**: To approve a task, reply in the chat with "Approved" or provide feedback on specific sub-tasks.
