# Display Service Implementation Plan (System State Registry)

## 1. Architectural Strategy: The "Whiteboard" Pattern
The `display_service` is a **pure observer** of the system state. It reads the `system_state_id_t` and other telemetry from the **System Status Registry** and renders the corresponding UI.

## 2. System Status Registry
Located in `components/system/include/system_registry.h`.

### Thread-Safe API
- **`esp_err_t system_registry_init()`**: Initializes internal Mutex.
- **`void system_registry_set_state(system_state_id_t state)`**: Updates the global system state.
- **`void system_registry_set_subtext(const char* text)`**: Updates status messages (e.g., "Starting WiFi...").
- **`void system_registry_update_wifi(int8_t level, bool connected)`**: Updates signal bars and MQTT status.
- **`void system_registry_update_motion(const char* motion, const char* balance)`**: Updates motion/tilt states.
- **`void system_registry_update_uptime(uint32_t uptime_sec)`**: Updates the system clock.
- **`void system_registry_get_snapshot(system_status_registry_t *out)`**: Copies the entire registry safely.

### API Integration (Producer Hooks)
| Service | Hook Location | API Call |
| :--- | :--- | :--- |
| **System Controller** | `state_transition()` | `system_registry_set_state(new_state)` |
| **Connectivity Manager** | WiFi/MQTT Event Handlers | `system_registry_update_wifi(bars, ok)` |
| **Motion Monitor** | `motion_task` loop (FSM transition) | `system_registry_update_motion(m_str, b_str)` |
| **Main Loop** | Periodic 1s timer/task | `system_registry_update_uptime(ticks/1000)` |

## 3. The 3-Layer Display Architecture

### Layer 1: Graphics Primitives (Internal)
- `display_draw_pixel`, `display_draw_string`, `display_draw_bitmap`, `display_draw_line`.
- Assets: `display_font.h` (5x7, 10x14), `display_icons.h` (WiFi bars, MQTT cloud).

### Layer 2: UI Components (Internal)
*Rendering logic for specific phases.*
- `ui_draw_header(wifi, mqtt)`: Renders signal strength and connection status.
- `ui_draw_footer(uptime)`: Renders uptime and heartbeat dot.
- `ui_draw_initializing_view(subtext)`: "STARTING DEVICE..." + specific boot step.
- `ui_draw_monitoring_view(motion, balance)`: Real-time elevator status.
- `ui_draw_configuring_view(subtext)`: "Configuring through webserver...".
- `ui_draw_error_view(fault, val)`: Full-screen inverse alert for emergencies.

### Layer 3: UI Manager
- **`display_refresh_task()`**: 15Hz FreeRTOS task.
- Logic:
    1. `system_registry_get_snapshot(&snapshot)`
    2. `display_clear()`
    3. `switch(snapshot.current_state)` -> Call corresponding Layer 2 View.
    4. `display_flush()` -> Burst write to SSD1306.

## 4. Implementation Steps
1. **Registry**: Implement `system_registry.h/c` in `components/system`.
2. **Assets**: Implement `display_font.h` (ASCII) and `display_icons.h`.
3. **Display Service**: Implement Layer 1 -> Layer 2 -> Layer 3.
4. **Integration**: Hook existing services into the Registry and verify the data flow.

---

# WiFi Provisioning System Implementation Plan (SoftAP + Web Server)

## 1. Objective
Implement a user-friendly WiFi provisioning system that allows the ESMU to boot into Access Point (AP) mode, host a web portal for entering home WiFi credentials, and persist these credentials in Non-Volatile Storage (NVS).

## 2. Key Components & Context
- **NVS Manager**: Storage for WiFi SSID/Password.
- **WiFi Service**: Supports both AP and STA modes.
- **Web Server**: Hosts the HTML form and handles credential submission.
- **Connectivity Manager**: Orchestrates the flow between Provisioning and Normal Operation.

## 3. Implementation Steps

### Phase 1: NVS Storage Implementation
1. **Create `nvs_storage` component**:
    - Implement `nvs_storage_init()` to initialize NVS flash.
    - Implement `nvs_storage_save_wifi_creds(ssid, password)`.
    - Implement `nvs_storage_load_wifi_creds(ssid, password, max_len)`.
    - Implement `nvs_storage_clear_wifi_creds()`.

### Phase 2: Web Server Component
1. **Create `web_server` component**:
    - Use `esp_http_server` component.
    - Implement `web_server_start()` and `web_server_stop()`.
    - Create a static HTML string for the provisioning form.
    - Implement a POST handler for `/config` to receive SSID/Password.
    - Signal the `connectivity_manager` when credentials are received.

### Phase 3: WiFi Service Enhancements
1. **Modify `wifi_sta.c`**:
    - Rename/Refactor to `wifi_manager.c` to support `WIFI_MODE_AP` and `WIFI_MODE_STA`.
    - Implement `wifi_manager_start_ap(const char *ssid, const char *password)`.
    - Ensure both modes can be switched gracefully.

### Phase 4: Connectivity Manager Orchestration
1. **Update `connectivity_manager.c` logic**:
    - **Startup**:
        - Try loading credentials from NVS.
        - If (not found): Start `wifi_manager` in AP mode + Start `web_server`.
        - If (found): Start `wifi_manager` in STA mode + Continue to MQTT.
    - **Provisioning Flow**:
        - Once `/config` is submitted, stop `web_server`, stop AP mode.
        - Save to NVS.
        - Start STA mode with new credentials.

### Phase 5: UI Integration
1. **Update `display_service`**:
    - Show "AP Mode: ESMU-Setup" on the OLED when in provisioning mode.
    - Show the IP address (`192.168.4.1`) for the user to visit.

## 4. Verification & Testing
1. **NVS Test**: Manually trigger save/load and verify persistence across reboots.
2. **AP Mode Test**: Verify the "ESMU-Setup" SSID appears on a phone/PC.
3. **Web Portal Test**: Connect to the AP, open `192.168.4.1`, and submit dummy credentials.
4. **Transition Test**: Verify the device successfully switches to STA mode after submission.

## 5. Migration & Rollback
- If provisioning fails, the device should time out and return to AP mode or keep old credentials.
- Add a "Reset to Defaults" mechanism (e.g., hold button for 10s) to clear NVS.
