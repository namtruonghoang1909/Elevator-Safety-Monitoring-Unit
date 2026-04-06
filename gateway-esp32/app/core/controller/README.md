# System Controller (Supervisor)

The **System Controller** is the high-level brain of the Gateway Node. It implements a Finite State Machine (FSM) that orchestrates the overall system lifecycle, handles asynchronous events, and manages global error states.

## 🚀 Key Features

- **Event-Driven Architecture**: Uses a FreeRTOS queue to handle events from various services asynchronously.
- **Central FSM**: Manages transitions between `IDLE`, `INITIALIZING`, `CONFIGURING`, `MONITORING`, and `ERROR` states.
- **Fault Management**: Responds to `SYSTEM_EVENT_ELEVATOR_FAULT_DETECTED` by triggering the `cellular_service` for SMS/Call alerts.
- **Boot Orchestration**: Triggers the `system_boot` sequence after hardware initialization is complete.

## 🏗️ State Machine (FSM)

| State | Transition Event | Action |
| :--- | :--- | :--- |
| **IDLE** | `SYSTEM_EVENT_BOOT` | Move to `INITIALIZING`, start boot sequence. |
| **INITIALIZING** | `SYSTEM_EVENT_INITIALIZED` | Move to `MONITORING` (Normal Operation). |
| **MONITORING** | `SYSTEM_EVENT_START_CONFIGURATION` | Move to `CONFIGURING` (Provisioning portal). |
| **CONFIGURING** | `SYSTEM_EVENT_DEVICE_CONFIGURATED` | Move back to `MONITORING`. |
| **ERROR** | `SYSTEM_EVENT_ERROR` | Fail-safe mode; remains until reset. |

## 🔧 Public API

- **`system_core_init()`**: Initializes the registry, hardware, and starts the supervisor task.
- **`system_report_event(event_id)`**: Thread-safe way for services to notify the controller of changes.
- **`system_report_error(error_id)`**: Triggers a global error state and updates logs.

## 📂 Dependencies
- `system_registry`: For state reporting.
- `cellular_service`: For triggering emergency alerts.
- `system_boot`: For service initialization sequence.
- `freertos`: For task and queue management.
