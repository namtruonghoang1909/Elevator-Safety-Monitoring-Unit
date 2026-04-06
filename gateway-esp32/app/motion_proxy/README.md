# Motion Proxy Service

The **Motion Proxy Service** acts as the Gateway's internal representative for the elevator's motion data. It listens to the CAN bus for messages from the Edge Node (STM32) and synchronizes them with the Gateway's `system_registry`.

## 🚀 Key Features

- **CAN Synchronization**: Listens for `CAN_ID_ELE_HEALTH`, `CAN_ID_ELE_EMERGENCY`, and `CAN_ID_EDGE_HEALTH` packets.
- **Registry Bridge**: Updates the thread-safe `system_registry` so the UI and Telemetry services can access the latest elevator metrics.
- **Emergency Forwarding**: Detects critical CAN emergency packets and reports them to the `system_controller` via `SYSTEM_EVENT_ELEVATOR_FAULT_DETECTED` to trigger cellular alerts (SMS/Calls).
- **Communication Watchdog**: Monitors Edge node heartbeats. If no heartbeat is received for 5 seconds, it marks the Edge node as offline and triggers a `COMMUNICATION LOST` error state.

## 🏗️ Architecture

| Module | Location | Responsibility |
| :--- | :--- | :--- |
| **Public API** | `motion_proxy.c` | External interface for initialization and health checks. |
| **Task Layer** | `src/core/task.c` | CAN listener task and watchdog monitor task. |

## 🔧 Integration

### Initialization
```c
motion_proxy_init();
```

### Health Check
```c
if (motion_proxy_is_alive(5000)) {
    // Edge node is communicating
}
```

## 📂 Dependencies
- `can_platform`: For low-level CAN bus access.
- `system_registry`: For global status reporting.
- `system_event`: For reporting critical faults to the controller.
- `esmu_protocol`: For packet structure definitions.
