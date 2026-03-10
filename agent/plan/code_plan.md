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
