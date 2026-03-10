# ESMU System Component Redesign

## 1. Do we need a System Component?
**YES.** Even though Display, Motion, and Connectivity are independent tasks, we need a "Central Brain" (Supervisor) for:
- **Global Error Handling**: If the MPU6050 task hangs, only a supervisor can decide to enter "Safe Mode."
- **Phase Transitions**: Orchestrating the move from Booting -> Config -> Monitoring.
- **Unified Event Bus**: Allowing services to communicate without being directly "wired" to each other.

## 2. Redesigned Architecture (The Supervisor Pattern)

### A. The Two-Stage Init (Solving the Race Condition)
Instead of a single `system_init`, use:
1. `controller_pre_init()`: Creates the `system_event_queue`. Guaranteed to run before any event is sent.
2. `system_start()`: Sends the `BOOT` event and then spawns the `system_task` to begin processing.

### B. Functional States
- `IDLE`: Power applied, no hardware touched yet.
- `INITIALIZING`: Hardware discovery and self-test.
- `CONFIGURING`: No WiFi credentials or user triggered manual setup.
- `CALIBRATING`: MPU6050 zero-point calculation (requires device to be flat).
- `MONITORING`: Normal production loop.
- `FAIL_SAFE`: Sensor lost or critical error. Display shows "SYSTEM HALTED."

### C. Folder Structure
```text
components/system/
├── include/
│   ├── system.h           (Public Orchestrator API)
│   ├── system_registry.h  (The Whiteboard)
│   ├── system_event.h     (Events: BOOT, WIFI_READY, FAULT, etc.)
│   └── system_error.h     (Error codes)
├── src/
│   ├── system.c           (The Orchestrator / Entry Logic)
│   ├── system_registry.c  (State storage)
│   └── controller/
│       ├── system_controller.c (The FSM Task)
│       └── system_supervisor.c (Watchdog for other service heartbeats)
```

## 3. The "Service Heartbeat" Implementation
Each service (Display, Motion, Connectivity) will report a "Heartbeat" to the `system_supervisor` every 1 second. 
- If the supervisor detects a service has "gone dark," it forces the system into `FAIL_SAFE` mode. 
- This is critical for an elevator where a frozen "IDLE" display could hide a real emergency.

## 4. Implementation Steps
1. Refactor `system_controller` to support `pre_init`.
2. Move credentials check into a dedicated "Provisioning" check in the `INITIALIZING` phase.
3. Implement the `FAIL_SAFE` transition in the FSM.
