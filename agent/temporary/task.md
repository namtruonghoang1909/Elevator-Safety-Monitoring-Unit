# Task: Modern Diagnostic Dashboard Implementation

## Objective
Transform the simple WiFi provisioning page into a live telemetry dashboard that shows real-time vibration, motion state, and network health.

## Todo List

### 1. Backend Implementation (ESP32)
- [ ] **Create `web_api_handlers.c`**:
    - [ ] Implement `web_api_status_json_handler` to serialize `system_status_registry_t` snapshot into a JSON string using `cJSON`.
    - [ ] Implement other API-related handlers if needed (e.g., config fetch).
- [ ] **Modify `web_server.c`**:
    - [ ] Declare the external status handler from `web_api_handlers.c`.
    - [ ] Define `status_uri` struct for `/api/status`.
    - [ ] Register `status_uri` in `web_server_start`.
- [ ] **Update `CMakeLists.txt`**:
    - [ ] Add `web_api_handlers.c` to the `SRCS` list.
- [ ] **Verification**:
    - [ ] Build and flash the firmware.
    - [ ] Access `http://<ip>/api/status` to verify JSON output.

### 2. Frontend Implementation (Web Assets)
- [ ] **Update `index.html`**:
    - [ ] Implement a **Single Page Application (SPA)** structure with two main views: `dashboard-view` and `config-view`.
    - [ ] Add a navigation bar (top or side) to switch between "Live Monitoring" and "System Settings".
    - [ ] Dashboard View: Responsive grid UI with "cards" for different metrics (Motion, Network, System).
    - [ ] Config View: Port existing WiFi/Phone setup form here.
- [ ] **Update `style.css`**:
    - [ ] Implement a professional "Dark Mode" dashboard styling.
    - [ ] Add styles for navigation tabs (Active/Inactive states).
    - [ ] Add styles for cards, progress bars, and status indicators.
- [ ] **Update `script.js`**:
    - [ ] Add view switching logic (show/hide sections based on navigation clicks).
    - [ ] Implement AJAX polling (using `fetch`) to get `/api/status` every 1000ms.
    - [ ] Update the DOM with received data.
- [ ] **Verification**:
    - [ ] Run `pio run --target uploadfs` to upload new assets.
    - [ ] Open the web interface and verify real-time updates.

### 3. Documentation & Cleanup
- [ ] Update `agent/context/roadmap.md` and `agent/context/checkpoint.md`.
- [ ] Update `gateway-esp32/middleware/connectivity/web_server/README.md` if it exists.
- [ ] Record the changes in `agent/context/memory.md`.
