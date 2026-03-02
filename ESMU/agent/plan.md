# ESMU Connectivity Manager Plan (Detailed API Spec)

## Overview
The `connectivity_manager` acts as the orchestrator for the entire network stack. It sits above `wifi_sta` and `mqtt_manager`, ensuring they work in harmony and handle edge cases (like weak signal) automatically.

---

## Detailed API Specifications

### 1. `esp_err_t connectivity_manager_init(const conn_mgr_config_t *config)`
**What it does:**
- Validates the provided WiFi and MQTT credentials.
- Initializes both the `wifi_sta` and `mqtt_manager` components internally.
- Sets up an internal FreeRTOS task (the "Network Orchestrator") that will manage the state transitions.
- Does **not** start the connections yet.

### 2. `esp_err_t connectivity_manager_start(void)`
**What it does:**
- Signals the internal Orchestrator task to begin the connection sequence.
- Calls `wifi_sta_connect()`.
- The task then enters a "Wait for IP" loop.

### 3. `bool connectivity_manager_is_ready(void)`
**What it does:**
- Returns `true` ONLY when:
    1. WiFi is connected and has a valid IP.
    2. RSSI is above a minimum threshold (e.g., -90dBm).
    3. MQTT has successfully handshaked with the broker.
- This is the primary function for `main.c` or the `system_controller` to check before publishing telemetry.

### 4. `int8_t connectivity_manager_get_rssi(void)`
**What it does:**
- Directly fetches and returns the current WiFi signal strength from the `wifi_sta` layer.
- Useful for the UI/Display to show a signal bar.

### 5. `conn_mgr_state_t connectivity_manager_get_state(void)`
**What it does:**
- Returns the current high-level state of the network stack:
    - `CONN_IDLE`: Not started.
    - `CONN_WIFI_ONLY`: Connected to AP, but MQTT not yet ready.
    - `CONN_FULL`: Everything is online.
    - `CONN_RECOVERING`: WiFi is up, but MQTT is in a retry/backoff loop.

---

## Internal Logic (The "Network Orchestrator" Task)

**The task will follow this loop every 1 second:**
1. **WiFi Check:** If WiFi is lost, immediately signal `mqtt_manager_stop()` and transition to `CONN_WIFI_ONLY`.
2. **Stability Guard:** Once WiFi reconnects, start a 5-second timer. Do NOT start MQTT until this timer expires.
3. **RSSI Watchdog:** If RSSI is extremely weak (<-92dBm), the manager will log a warning and potentially pause heavy MQTT traffic.
4. **MQTT Lifecycle:** If WiFi is stable and state is `CONN_WIFI_ONLY`, call `mqtt_manager_start()`.

---

## Next Steps
1. Add `connectivity_manager` to `agent/task.md` as Task 04.
2. Update `agent/todo.md` with the phased implementation steps.
