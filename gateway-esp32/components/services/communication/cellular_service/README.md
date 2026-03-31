# Cellular Service

The **Cellular Service** is a high-level background service that manages the life-cycle of the A7680C 4G LTE module. It abstracts the raw AT commands into a robust state machine and provides thread-safe APIs for emergency communication.

## 🚀 Key Features

- **Automated FSM**: Handles the transition from initialization to network searching and registration.
- **Background Monitoring**: Periodically (10s) checks signal strength (`CSQ`) and network status (`CREG`).
- **Health Reporting**: Updates the `system_registry` with real-time cellular metrics for UI and Telemetry.
- **Fault Recovery**: Automatically triggers hardware resets via the `sim_a7680c` driver if communication fails or the network is lost repeatedly.
- **Emergency APIs**: Simple, thread-safe functions for sending SMS alerts and initiating voice calls.

## 🏗️ State Machine (FSM)

| State | Description |
|-------|-------------|
| **IDLE** | Service is waiting to be started. |
| **INITIALIZING** | Syncing baud rate, disabling echo, and configuring radio mode. |
| **SEARCHING** | Actively waiting for SIM registration on the LTE/GSM network. |
| **READY** | Registered and signal strength is sufficient for operation. |
| **RECOVERING** | Executing hardware reset after repeated failures. |

## 🔧 Integration

### Initialization
The service should be initialized after the system registry and hardware (UART) are ready.
```c
cellular_service_init();
cellular_service_start();
```

### Emergency Alerting
```c
if (cellular_service_get_state() == CELLULAR_STATE_READY) {
    cellular_service_send_sms("+84123456789", "ALERT: Elevator Free Fall!");
}
```

## 📂 Dependencies
- `sim_a7680c`: Low-level driver for AT command execution.
- `system_registry`: For global status reporting.
- `bsp_uart`: For thread-safe UART communication.
