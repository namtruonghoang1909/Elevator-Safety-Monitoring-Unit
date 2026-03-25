# ESMU Project Memory Log

## Summary of Project Progress (March 2026)

This log summarizes the development of the Elevator Safety Monitoring Unit (ESMU), focusing on hardware drivers, connectivity, and build system integrity.

### 1. Core Drivers and Platform (I2C)
- **i2c_platform**: Implemented a modern, thread-safe I2C master abstraction using ESP-IDF's new `i2c_master.h` driver. Supports multi-bus management and dynamic device registration.
- **mpu6050**: Developed a comprehensive 6-axis driver with raw/scaled data reading, temperature conversion, and magnitude calculation.
- **ssd1306**: Implemented OLED display primitives.

### 2. Connectivity Stack & Manager
- **Components**: Developed `wifi_sta` (WiFi station) and `mqtt_manager` (MQTT wrapper) with auto-reconnect logic and state management.
- **Connectivity Manager**: An orchestrator that manages WiFi and MQTT lifecycles together.
- **Refactoring**: Moved telemetry logic into the manager and verified it with JSON payloads.
- **Verification**: Fully tested via `test/test_connectivity/test_connectivity.c`.

### 3. MPU6050 Interactive Verification Suite (Task 06)
- **Unity Test Suite**: Created `test/test_mpu6050/test_mpu6050.c` for hardware-in-the-loop (HIL) testing.
- **Interactive Logic**: 
    - Used **GPIO 15** for LED signaling (Blink = WAIT, OFF = SAMPLING).
    - Verified **Scaling & FSR**: Tested Z-axis readings at 2G and 16G ranges.
    - Verified **Physical Vectors**: Verified gravity magnitude (1.0g +/- 0.1g) and Z-axis polarity (Flat vs. Upside-down).
    - Verified **Robustness**: Proved the driver's auto-recovery logic can detect and repair register corruption in `GYRO_CONFIG`.
- **Documentation**: Added `test/test_mpu6050/README.md` with detailed interaction protocols.

### 4. Build System & Dependency Fixes
- **Issue**: `system.c` was failing to include `nvs_flash.h` during component-only test builds.
- **Fix**: Updated `components/system/CMakeLists.txt` to include `nvs_flash`, `esp_common`, `log`, `freertos`, and `esp_system` in the `REQUIRES` list.
- **Status**: Verified that the entire project compiles successfully in both the standard build and the Unity test environment.

### 5. SSD1306 Interactive Verification Suite (March 2026)
- **Unity Test Suite**: Created `test/test_ssd1306/test_ssd1306.c` for hardware-in-the-loop (HIL) testing.
- **Automated Verification**:
    - Verified **Initialization**: Successful I2C bus and device registration.
    - Verified **Control Commands**: Confirmed contrast adjustment, display inversion, and ON/OFF switching.
    - Verified **Robustness**: Proved the driver's bounds-handling logic prevents crashes when attempting to draw outside the 128x64 display area.
- **Visual Verification**:
    - **Checkerboard Pattern**: Confirmed no dead pixels and correct page mapping.
    - **Fill/Clear**: Verified full-screen white (0xFF) and black (0x00) states.
    - **Horizontal Scrolling**: Verified hardware scrolling (Right) and stop functionality.
- **Integration**: Updated `agent/context/roadmap.md` and `agent/temporary/task.md` to reflect the completed verification.

### 6. Motion Monitor Service Implementation (Task 06)
- **Component**: Created `components/services/motion_monitor`.
- **Filtering**: Implemented **EMA (Exponential Moving Average)** for 6-axis data to eliminate high-frequency motor noise.
- **Physics Engine**:
    - **Linear Z**: Isolation of vertical force by subtracting a calibrated 1.0g gravity baseline.
    - **Shake Magnitude**: Euclidean norm of the horizontal (XY) plane for vibration detection.
- **State Machine**:
    - Implemented a **debounced logical state engine** with 500ms transition windows.
    - Detects: `STATIONARY`, `MOVING_UP`, `MOVING_DOWN`, and `ACCELERATING`.
- **Full-Scale Adjust**: Configured to **±8g** to ensure headroom for emergency stop impact detection.
- **Build Fix**: Replaced `esp_timer_get_time()` with FreeRTOS `xTaskGetTickCount()` to resolve environment-specific `esp_timer.h` dependency issues.
- **Verification**: Created `test/test_motion_monitor/test_motion_monitor.c`. Verified build success with `src/main.c`.
- **Refactoring (March 2026)**:
    - Renamed `motion_monitor_get_state` to `motion_monitor_get_motion` for better semantic clarity.
    - Added `motion_monitor_get_equilibrium` to provide direct access to the `balance_state_t` (tilt) metrics.
    - Updated `test/test_motion_monitor/test_motion_monitor.c` with comprehensive verification of the new API.

### 7. Display Service & UI Enhancement (March 2026)
- **Component**: `components/services/display`.
- **UI Architecture**: Implemented a layered UI system with `display_primitives` (Layer 1), `display_ui_components` (Layer 2), and `display_service` (Layer 3).
- **Monitoring View**: 
    - Created an interactive **Elevator Visualization** that tilts based on the physical balance of the unit (LEFT/RIGHT).
    - Integrated motion arrows (UP/DOWN/IDLE) and health/balance text telemetry.
- **System Feedback**: Added a Status Bar with WiFi signal bars and MQTT connection status icons.
- **Fault Display**: Implemented an inverse-video "EMERGENCY" overlay for critical system states.

### 8. Debugging Standards & Error Handling (March 2026)
- **Compliance**: Updated all major service components to follow the "ESMU Debugging Rules".
- **Logging**: Replaced silent `ESP_ERROR_CHECK` macros with explicit `esp_err_t` checks and `ESP_LOGE` for better field diagnostics.
- **Robustness**: Implemented rate-limited error logging (every 100 frames) in high-frequency display tasks to prevent log flooding while maintaining visibility of persistent I2C failures.
- **Files Updated**: `system_controller.c`, `wifi_sta.c`, `display_service.c`, `connectivity_manager.c`, `mqtt_manager.c`.

### 9. Code Cleanup & Documentation Update (March 2026)
- **Refactoring**: Renamed internal files in `display` and `motion_monitor` components to remove redundant prefixes (`display_`, `motion_`). This improved code readability and clarified internal vs. public API boundaries.
- **CMake Alignment**: Updated `CMakeLists.txt` for both components to reflect the new file structure.
- **Comprehensive Docs**: Updated all existing READMEs and created missing documentation for `mpu6050`, `ssd1306`, `i2c_platform`, and `connectivity` stack. Added `agent/README.md` to explain agent metadata.
- **UI Refinement**: Moved MQTT icon to the header's left side (next to WiFi) and centered "ESMU" in the footer.

### 10. WiFi Provisioning & UI Refinement (March 2026)
- **Component**: Created `components/services/connectivity/web_server`.
- **SoftAP Portal**: Implemented a web-based configuration interface served at `192.168.4.1` when the device enters AP mode (`ESMU-Setup`).
- **URL Decoding**: Fixed connection issues with SSIDs containing spaces by implementing a custom URL decoder.
- **Hardware Toggle**: Enhanced the GPIO 15 logic to allow **bidirectional switching**:
    - STA -> AP: Hold 5s to configure.
    - AP -> STA: Hold 5s to cancel and return to normal operation.
- **Artistic UI**:
    - **Cloud Icon**: Implemented a custom 16x8 organic cloud based on a specific binary bitmap (3 bumps up, 2 bumps down).
    - **WiFi Icon**: Reverted to classic vertical bars for maximum readability while maintaining the "Wi-Fi:" label in Config Mode.
- **Persistence**: Integrated with `nvs_storage` to ensure credentials survive power cycles.

### 11. CAN Platform & Inter-MCU Communication (March 2026)
- **Component**: `components/platform/can_platform`.
- **Abstraction**: Implemented a thread-safe CAN (TWAI) abstraction layer supporting Normal, No-ACK, and Loopback modes.
- **Self-Test Solution**: Implemented "Software Loopback" using the ESP32 GPIO Matrix (`esp_rom_gpio_connect_in_signal`) to route TX signal internally to RX input.
- **Verification**: Successfully transmitted and received 11-bit CAN frames at 500kbps on GPIO 12/13.
- **End-to-End**: Verified physical communication between STM32 and ESP32 at 500kbps using MCP2551 transceivers and mandatory 120-ohm termination resistors.

### 12. STM32 Edge Node Porting (March 2026)
- **SSD1306 Driver**: Successfully ported the ESP32 SSD1306 driver to the STM32 HAL framework. Maintained API symmetry using `bsp_i2c` abstractions.
- **Edge Logger**: Implemented a local OLED-based "Serial Monitor" for the STM32. Supports multi-line scrolling and `edge_logger_printf` for real-time debugging without a PC.
- **Fixes**: 
    - Resolved `HardFault` by increasing `defaultTask` stack size to 512 words (2KB) to support heavy `vsnprintf` operations.
    - Fixed OLED initialization jitters by moving init into the FreeRTOS task after the scheduler starts.
    - Synchronized CAN timing (500kbps) between nodes (STM32: Prescaler 1, BS1 12, BS2 3 @ 8MHz HSI).

### 13. Edge Telemetry & Multi-level Health Status (March 2026)
- **Component**: Created `edge_telemetry` service on STM32 Edge Node.
- **Protocol Update**: 
    - Updated `ele_health_t` to include a `health_status` field.
    - Added `health_status_t` enum (STABLE, WARNING, EMERGENCY).
- **Functionality**:
    - Implemented periodic CAN broadcasting: `ele_health_t` (100ms), `edge_heartbeat_t` (1s), and `test_packet_t` (1s).
    - Integrated multi-level health logic into `motion_monitor` (e.g., Warning on high vibration).
    - Updated ESP32 `system_registry` to handle the new health status and trigger the **Full Alert View** during `HEALTH_EMERGENCY`.
- **Debugging**: Added 1Hz "Visual Heartbeat" LEDs to both nodes (STM32: PC13, ESP32: GPIO 4) for rapid state confirmation.
- **Verification**: Verified protocol alignment between STM32 (broadcast) and ESP32 (proxy receiver).

### 14. ESP32 Gateway System Component Refactor (March 22, 2026)
- **What**: Reorganized the `system` component into a functional subdirectory structure (`core/`, `boot/`, `registry/`, `storage/`, `services/`) and co-located header files with their source files.
- **Where**: `gateway-esp32/components/system/`
- **Why**: To align with "ESMU Coding Rules" for file organization, improve maintainability, and clarify module responsibilities by separating FSM logic from hardware/service initialization.
- **Result**: Successfully verified the new structure with a clean build on the Gateway ESP32 node. Header visibility was resolved by using `PUBLIC_INCLUDE_DIRS` (temporarily using `INCLUDE_DIRS` for testing, then successfully verified).

### 15. SPI Infrastructure & ST7789 Planning (March 22, 2026)
- **What**: Drafted a comprehensive implementation plan for a thread-safe SPI BSP and an ST7789 driver for the Gateway's color display upgrade.
- **Where**: `agent/plan/code_plan.md` and `agent/temporary/task.md`.
- **Status**: Ready for implementation in the next session.

---

## Current Project State
- **Drivers Layer**: [COMPLETE] i2c_platform, mpu6050, ssd1306, can_platform (ESP32/STM32).
- **Connectivity Layer**: [COMPLETE] wifi_manager, mqtt_manager, connectivity_manager, web_server.
- **Service Layer**: 
    - Motion Monitor: [COMPLETE]
    - Display Service: [PENDING MIGRATION TO ST7789]
    - Fault Detector: [PENDING]
    - Edge Logger (STM32): [COMPLETE]
- **Distributed System**:
    - Shared Protocol: [COMPLETE] Verified `test_packet_t` and ID `0x7FF`.
    - CAN Communication: [VERIFIED] End-to-end hardware success.

- **Next Session Focus**:
- **MPU6050 STM32 Porting**: Move the MPU6050 driver and `motion_monitor` logic from ESP32 to STM32.
- **ST7789 Migration**: Implement SPI-based ST7789 driver for the ESP32 Gateway.
- **Fault Detector**: Implement real-time anomaly detection on the Edge node.

### 16. Telemetry Component Naming Consistency (March 24, 2026)
- **What**: Renamed the `telemetry` component directory to `telemetry_service` and updated all internal dependencies.
- **Where**: `gateway-esp32/components/services/communication/telemetry_service`
- **Why**: Resolved a build failure where `motion_proxy` required `telemetry_service` but the component was named `telemetry` by the ESP-IDF build system (matching its folder name). Standardized the naming across `CMakeLists.txt` and source files.
- **Result**: Successfully verified with a full project build on the Gateway ESP32 node.

### 17. Elevator Fault Data Flow & Hysteresis Fix (March 25, 2026)
- **What**: Resolved a discrepancy where the ESP32 Gateway reported constant faults while the STM32 Edge OLED appeared healthy.
- **Where**: 
    - `edge-stm32/App/modules/services/motion_monitor/motion_monitor.c`
    - `gateway-esp32/components/system/registry/system_registry.c`
- **Why**: 
    - **STM32**: Vibration thresholds were too sensitive (5.0 deg/s) and calculated from raw, unfiltered gyro data, causing noise spikes to trigger emergency states. The 2Hz OLED logging missed these 10ms spikes.
    - **ESP32**: The system registry failed to reset the `fault_active` and `current_state` flags when receiving a `HEALTH_STABLE` status.
- **Fixes**:
    - **STM32**: Increased vibration thresholds (MED: 5.0, HIGH: 15.0), applied EMA filtering to vibration magnitude (filt_vib), implemented a 5-cycle (50ms) debounce for emergency entry/exit, and updated OLED logging to show the "worst" health status observed in each 500ms window.
    - **ESP32**: Updated `system_registry_update_from_protocol_health` to reset fault and error states upon receiving `HEALTH_STABLE`.
- **Result**: Improved system stability by eliminating noise-induced CAN spam and ensured the local display accurately reflects transient faults.

### 18. MQTT Telemetry Refactor & Protocol Expansion (March 25, 2026)
- **What**: Refactored MQTT telemetry messages for clarity and expanded the shared protocol to include motion state.
- **Where**: 
    - `shared/can_protocol/protocol_types.h` / `protocol_packets.h`
    - `edge-stm32/App/modules/services/edge_telemetry/edge_telemetry.c`
    - `gateway-esp32/components/services/communication/telemetry_service/telemetry_service.c`
    - `gateway-esp32/components/system/registry/system_registry.c` / `system_registry.h`
- **Why**: The previous MQTT messages lacked clarity (no prefixes) and missed critical "system integrity" data (Gateway uptime, Edge connectivity status). Motion state was also missing from the CAN health packet.
- **Fixes**:
    - **Protocol**: Added `motion_state_t` to the shared protocol and included it in the `ele_health_t` CAN packet (8-byte packed).
    - **STM32**: Updated `edge_telemetry` to send the current elevator motion state (IDLE, UP, DOWN, etc.) via CAN.
    - **ESP32**: 
        - Updated `system_registry` to extract and store the raw motion state from CAN.
        - Refactored `telemetry_service` to combine periodic "health" and "node" messages into a single **Unified Telemetry** JSON payload.
        - Added prefixes (`ele_`, `edge_`, `gw_`) to all MQTT fields for source clarification.
        - Expanded the payload to include: `ele_motion`, `edge_connected`, `gw_wifi_rssi`, `gw_uptime`, and `gw_state`.
- **Result**: Reduced MQTT overhead and provided a more professional, comprehensive "single source of truth" message for cloud monitoring.

### 19. Telemetry Cleanup & Fault Messaging Fix (March 25, 2026)
- **What**: Removed redundant `max_tilt` data and fixed a bug where fault messages were overwritten by connectivity status.
- **Where**: 
    - `shared/can_protocol/protocol_packets.h`
    - `edge-stm32/App/modules/services/motion_monitor/motion_monitor.h`
    - `edge-stm32/App/modules/services/edge_telemetry/edge_telemetry.c`
    - `gateway-esp32/components/services/communication/telemetry_service/telemetry_service.c`
    - `gateway-esp32/components/system/registry/system_registry.c` / `system_registry.h`
- **Why**: 
    - `max_tilt` was deemed unnecessary for the current telemetry requirements.
    - `ele_fault_msg` was pulling from `sub_status`, which is shared with connectivity updates (like "Connected"), causing the fault description to be lost.
- **Fixes**:
    - **Protocol**: Removed `max_tilt` from `ele_health_t` and added reserved padding.
    - **STM32**: Removed `max_tilt` from `motion_metrics_t` and stopped broadcasting it via CAN.
    - **ESP32**: 
        - Removed `max_tilt` from the system registry and MQTT payload.
        - Implemented `get_fault_msg()` in `telemetry_service.c` to map fault codes (e.g., 3 -> "EMERGENCY STOP") directly to the MQTT `ele_fault_msg` field, bypassing the shared `sub_status`.
- **Result**: Cleaner telemetry payloads and guaranteed accuracy for fault reporting in the cloud.

