# System Layer

The System Layer provides the "Brain" and "Whiteboard" of the ESMU, managing global state transitions, event dispatching, and centralized telemetry storage.

## Components

### 1. System Controller (`controller/system_controller.c`)
The Controller is a supervisor task that manages the system's Finite State Machine (FSM). It responds to events (Boot, Fault, Config) and orchestrates transitions between high-level states.

#### States:
- `SYSTEM_STATE_INITIALIZING`: Booting hardware and services.
- `SYSTEM_STATE_MONITORING`: Normal operation, analyzing elevator motion.
- `SYSTEM_STATE_CONFIGURING`: Local web server active for provisioning.
- `SYSTEM_STATE_ERROR`: Critical fault detected, emergency procedures active.

### 2. System Registry (`system_registry.c`)
The Registry is a thread-safe "Whiteboard" where all services post their status and metrics. The Display Service reads from this registry to render the UI.

#### Telemetry Tracked:
- WiFi Signal Level (0-4 bars).
- MQTT Connection Status.
- Motion State ("IDLE", "UP", "DOWN").
- Balance State ("STABLE", "TILT LEFT", etc.).
- Elevator Health ("GOOD", "SICK", "CRITICAL").
- System Uptime.

## Event System

Services communicate with the Controller via `system_report_event()` and `system_report_error()`.

| Event | Source | Impact |
| :--- | :--- | :--- |
| `SYSTEM_EVENT_BOOT` | `main.c` | Starts initialization. |
| `SYSTEM_EVENT_INITIALIZED` | Controller | Transitions to Monitoring. |
| `SYSTEM_EVENT_FAULT_DETECTED` | Fault Detector | Transitions to Error state. |

## Configuration

System-wide hardware pins and fixed thresholds are defined in `include/system_config.h`.
