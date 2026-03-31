# Cellular Service Design (SIM A7680C)

This document outlines the high-level design and flow for the `cellular_service` on the ESMU Gateway.

## 1. Overview
The `cellular_service` is an event-driven background service that manages the SIM module's lifecycle. It acts as a bridge between the raw `sim_a7680c` driver and the rest of the system (e.g., `connectivity_manager`, `alert_system`).

## 2. State Machine (FSM)

| State | Description | Transition Trigger |
|-------|-------------|--------------------|
| **IDLE** | Module is powered off or not initialized. | `cellular_service_start()` |
| **INITIALIZING** | Syncing baud, disabling echo, getting basic info (IMEI/IMSI). | `sim_a7680c_init()` Success -> **SEARCHING** |
| **SEARCHING** | Waiting for network registration (CREG/CEREG). | Registered -> **READY**; Timeout -> **RECOVERING** |
| **READY** | Registered and signal strength is adequate. | Signal Lost -> **SEARCHING**; Error -> **RECOVERING** |
| **RECOVERING** | Attempting to fix issues (Reset, AT+CFUN=1). | Success -> **INITIALIZING**; Fail Max -> **ERROR** |
| **ERROR** | Permanent hardware failure or SIM missing. | Manual Reset |

## 3. Recovery Strategies

### Scenario A: SIM Not Inserted
- **Detection**: `IMSI` retrieval fails or `CPIN?` returns error.
- **Action**: Transition to **ERROR** state after 3 retries. Update `system_registry` to notify UI.

### Scenario B: Registration Denied (e.g., No Balance)
- **Detection**: `CREG` status is `3` (Registration denied) or `0` (Searching too long).
- **Action**: Log warning, wait 60s, then retry `AT+CFUN=1,1` (Full reset of radio stack).

### Scenario C: Module Hang (UART Timeout)
- **Detection**: Mutex timeout or consecutive UART failures in BSP.
- **Action**: Pulse `PWRKEY` (Hardware Reset) via `sim_a7680c_hw_reset()`. Transition to **INITIALIZING**.

## 4. Background Polling (Every 10s)
The service task will periodically:
1. Check `CSQ` (Signal Quality).
2. Check `CREG/CEREG` (Registration).
3. Check `CBC` (Battery/Voltage) and `CPMUTEMP` (Temperature).
4. Update `system_registry` for UI/Telemetry.

## 5. Event Reporting
The service will emit system events for other modules to consume:
- `CELLULAR_EVENT_REGISTERED`: System can now send SMS/MQTT over LTE.
- `CELLULAR_EVENT_DISCONNECTED`: Failover to other connectivity if available.
- `CELLULAR_EVENT_SMS_RECEIVED`: (Future) Handle remote commands.

## 6. API Surface (Proposed)
```c
esp_err_t cellular_service_init(void);
esp_err_t cellular_service_start(void);
cellular_status_t cellular_service_get_status(void);
esp_err_t cellular_service_send_sms(const char *num, const char *msg);
esp_err_t cellular_service_emergency_call(void);
```
