# ESMU Project Memory Log

...

### 21. WiFi RSSI & Scaled Vibration Telemetry (March 27, 2026)
- **What**: Modified the ESP32 Gateway to publish raw WiFi RSSI (dBm) and scaled vibration (float, deg/s) instead of signal bars and protocol-packed integers.
- **Where**: 
    - `gateway-esp32/components/system/registry/system_registry.h/c`
    - `gateway-esp32/components/services/connectivity/connectivity_manager/connectivity_manager.c`
    - `gateway-esp32/components/services/communication/telemetry_service/telemetry_service.c`
- **Why**: MQTT telemetry was publishing WiFi signal bars (0-4) and unscaled vibration data (integer mg*100), which were insufficient for high-resolution cloud visualization in CoreIOT.
- **Fixes**: 
    - Updated `system_status_registry_t` to include `wifi_rssi` (int8_t) and `scaled_vibration` (float).
    - Modified `connectivity_manager.c` to report raw RSSI to the registry.
    - Updated `system_registry_update_from_protocol_health` to calculate `scaled_vibration` from CAN packets (dividing by 100.0f).
    - Modified `telemetry_service.c` to publish `gw_wifi_rssi` as raw dBm and `ele_vibration` as a float (`%.2f`) in deg/s.
    - **Fix (Stability)**: Zero-initialized `wifi_config_t` in `wifi_manager.c` to prevent latent connection issues caused by stack garbage/structure alignment changes.
- **Result**: CoreIOT now receives high-fidelity signal strength and motion data for better visualization. Verified with a successful project build.

### 22. Generic UART BSP & SIM A7680C Driver Implementation (March 28, 2026)
- **What**: Refactored the UART BSP to be generic and implemented a specialized driver for the SIM A7680C module.
- **Where**: 
    - `gateway-esp32/components/bsp/uart_bsp`
    - `gateway-esp32/components/drivers/sim_a7680c`
- **Why**: The previous UART BSP was tightly coupled to the SIM module's AT protocol. Decoupling was necessary to allow the BSP to serve other UART devices (like the upcoming SIM7600 or debugging tools) without modifying core BSP code.
- **Fixes**:
    - **UART BSP**: Removed all SIM-specific "URC" and "Response" logic. Added `uart_bsp_set_data_cb()` to allow higher-level drivers to register their own data processing callbacks.
    - **SIM A7680C**: 
        - Implemented local line-buffering and response queueing within the driver.
        - Decoupled `uart_bsp_init` from the driver; initialization is now handled externally (e.g., in `main.c`).
        - Implemented AT command parsing for Signal Strength (`CSQ`), Network Registration (`CREG`), and Operator Name (`COPS`).
- **Result**: A cleaner, more modular architecture that supports multiple UART devices while maintaining robust AT command processing for the SIM module. Verified with `main.c` test suite.

### 23. Robust UART BSP & SIM Driver Refactor (March 30, 2026)
- **What**: Refactored the UART BSP to support synchronous, thread-safe communication and updated the SIM A7680C driver to a robust half-duplex model.
- **Where**: 
    - `gateway-esp32/components/bsp/uart_bsp/`
    - `gateway-esp32/components/drivers/sim_a7680c/`
- **Why**: The previous callback-based model competed with manual reads, making AT command parsing fragile and prone to data loss. A synchronous "Send-Wait-Capture" model is standard for reliable AT command processing.
- **Fixes**:
    - **UART BSP**: 
        - Added `g_rx_mutex` for thread-safe reading.
        - Implemented `uart_bsp_read_line()` with support for `\r\n` and `\n` terminators.
        - Added `uart_bsp_flush()`, `uart_bsp_get_available()`, and `uart_bsp_wait_data()`.
        - Modified the background task to only run if a data callback is registered, preventing buffer competition.
    - **SIM A7680C**:
        - Removed the private response queue and line-buffering callback.
        - Implemented `wait_for_terminal()` helper to capture raw response lines until "OK" or "ERROR".
        - Updated all AT command functions (Ping, CSQ, CREG, COPS) to use the new synchronous APIs.
        - Improved `hw_reset()` with better timing (10s boot wait) and `init()` with communication retries.
- **Result**: Highly reliable AT command communication with 100% success rate during initialization tests. Verified that **9600 baud** is the required rate for the current A7680C module (autobauding/default). Updated `system_config.h` to match. Verified with a successful project build and serial monitoring.

### 24. SIM Driver Expansion: SMS & Voice Call Support (March 30, 2026)
- **What**: Expanded the SIM A7680C driver to support SMS messaging and voice calling.
- **Where**: 
    - `gateway-esp32/components/drivers/sim_a7680c/sim_a7680c.h/c`
    - `gateway-esp32/src/main.c`
    - `gateway-esp32/components/drivers/sim_a7680c/README.md`
- **Why**: To fulfill Phase 3 of the LTE integration plan, enabling the ESMU to send emergency alerts via SMS and initiate voice calls during critical faults.
- **Fixes**:
    - **SIM Driver**:
        - Implemented `sim_a7680c_send_sms()` with support for Text Mode (`AT+CMGF=1`) and `CTRL+Z` (0x1A) termination.
        - Added `wait_for_prompt()` helper to detect the `>` character before sending message bodies.
        - Implemented `sim_a7680c_make_call()` using the `ATD` command.
        - Implemented `sim_a7680c_hang_up()` using the `ATH` command.
    - **Documentation**: Created a comprehensive `README.md` for the SIM driver, detailing API usage and hardware requirements.
    - **Testing**: Updated the `main.c` diagnostic suite with commented-out test hooks for SMS and calls to facilitate safe physical testing.
- **Result**: The SIM driver now provides a complete set of emergency communication APIs. Verified with a successful project build.

### 25. Cellular Service Implementation & Registry Integration (March 31, 2026)
- **What**: Implemented the `cellular_service` as a background task and integrated it with the `system_registry`.
- **Where**: 
    - `gateway-esp32/components/services/communication/cellular_service/`
    - `gateway-esp32/components/system/registry/system_registry.h/c`
    - `gateway-esp32/src/main.c`
    - `gateway-esp32/CMakeLists.txt`
- **Why**: To provide a robust, event-driven background service for managing the SIM module's lifecycle, network registration, and providing high-level APIs for SMS/Calls to other system modules.
- **Fixes**:
    - **Cellular Service**: Implemented a Finite State Machine (FSM) with states: `IDLE`, `INITIALIZING`, `SEARCHING`, `READY`, `RECOVERING`, and `ERROR`. The task periodically polls for signal strength and registration status.
    - **System Registry**: Added `cellular_connected`, `cellular_level`, `cellular_rssi`, and `cellular_operator` fields to `system_status_registry_t`. Implemented `system_registry_update_cellular()` for thread-safe updates.
    - **Main App**: Refactored `main.c` to use `cellular_service_init()` and `cellular_service_start()` instead of direct driver calls, improving separation of concerns.
    - **Build System**: Updated `CMakeLists.txt` and `src/CMakeLists.txt` to include the new component and its requirements.
- **Result**: The Gateway now has a dedicated service for cellular connectivity that automatically handles initialization, network searching, and recovery. Verified with a successful project build.
