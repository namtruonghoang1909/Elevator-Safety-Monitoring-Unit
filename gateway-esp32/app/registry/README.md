# System Registry (Whiteboard)

The **System Registry** is the central "Whiteboard" for the Gateway Node. It provides a thread-safe, centralized repository for system functional states, connectivity metrics, and elevator telemetry received from the Edge Node.

## 🚀 Key Features

- **Thread-Safe Access**: All updates and snapshot retrievals are protected by a Mutex.
- **Centralized State**: Acts as the single source of truth for UI, Telemetry, and Connectivity components.
- **Protocol Integration**: Provides specific parsers for CAN protocol packets (`ele_health_t`, `edge_heartbeat_t`, `ele_emergency_t`).
- **Telemetry Snapshots**: Allows services to take a consistent snapshot of the entire system state for reporting or visualization.

## 🏗️ Registry Data (`system_status_registry_t`)

| Field | Description |
| :--- | :--- |
| `current_state` | Global functional state (IDLE, MONITORING, CONFIGURING, ERROR). |
| `wifi_level` | Signal bars (0-4) and raw RSSI. |
| `cellular_level` | Signal bars (0-4) and raw CSQ (0-31). |
| `emergency_phone` | User-configured contact number for SMS/Calls. |
| `motion_state` | Elevator status (IDLE, UP, DOWN, SHAKE, FREEFALL). |
| `elevator_health` | System health assessment (GOOD, WARNING, CRITICAL). |
| `fault_active` | Boolean trigger for emergency UI overlays and alerts. |
| `last_fault_code` | Raw code for the most recent fault detected. |

## 🔧 Public API

- **`system_registry_init()`**: Initializes the mutex and clears the registry.
- **`system_registry_set_state(state)`**: Updates the global system FSM state.
- **`system_registry_update_wifi(...)`**: Updates WiFi and MQTT connectivity info.
- **`system_registry_update_cellular(...)`**: Updates cellular registration and signal strength.
- **`system_registry_update_emergency_phone(phone)`**: Updates the emergency contact number.
- **`system_registry_get_snapshot(out)`**: Thread-safe copy of the entire registry.
- **`system_registry_update_from_protocol_*`**: Specialized parsers for CAN messages.

## 📂 Dependencies
- `freertos`: For Mutex protection.
- `esmu_protocol`: For packet structure definitions.
