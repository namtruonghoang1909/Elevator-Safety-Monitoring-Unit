# ESMU Connectivity Manager Implementation (Task 04) ✅ COMPLETED

## Task
Implement `connectivity_manager` to coordinate `wifi_sta` and `mqtt_manager`, ensuring stable connections in poor WiFi environments.

---

## Phase 1: Create Component Structure ✅ COMPLETED
- [x] Create `components/services/connectivity/connectivity_manager/include/` directory.
- [x] Create `components/services/connectivity/connectivity_manager/connectivity_manager.c`.
- [x] Create `components/services/connectivity/connectivity_manager/include/connectivity_manager.h`.
- [x] Create `components/services/connectivity/connectivity_manager/CMakeLists.txt`.

## Phase 2: Implement Header (connectivity_manager.h) ✅ COMPLETED
- [x] Define `connectivity_config_t` (WiFi and MQTT config combined).
- [x] Define `connectivity_state_t` enum.
- [x] Declare public APIs:
  - `esp_err_t connectivity_manager_init(const connectivity_config_t *config)`
  - `esp_err_t connectivity_manager_start(void)`
  - `bool connectivity_manager_is_ready(void)`
  - `int8_t connectivity_manager_get_rssi(void)`
  - `connectivity_state_t connectivity_manager_get_state(void)`

- NOTICE: uses connectivity_config_t instead of conn_mgr_config_t, connectivity_state_t instead of conn_mgr_state_t

## Phase 3: Implement Core Logic (connectivity_manager.c) ✅ COMPLETED
- [x] Implement `connectivity_manager_task` (the orchestrator).
- [x] Add the 5-second WiFi stability timer.
- [x] Implement logic to automatically start/stop MQTT based on WiFi/RSSI state.
- [x] Add detailed logging for state transitions.

## Phase 4: Component Integration ✅ COMPLETED
- [x] Create `CMakeLists.txt` for the component.
- [x] Add the component to root `CMakeLists.txt`.

## Phase 5: Refactor main.c ✅ COMPLETED
- [x] Replace individual WiFi/MQTT init calls with `connectivity_manager`.
- [x] Test the automatic recovery by simulating a disconnect or moving to a weak signal area.

## Phase 6: Verification ✅ COMPLETED
- [x] Build the project using PlatformIO.
- [x] Verify that MQTT only starts after WiFi is stable.
