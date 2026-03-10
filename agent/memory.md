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
- **Integration**: Updated `agent/roadmap.md` and `agent/todo.md` to reflect the completed verification.

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

---

## Current Project State
- **Drivers Layer**: [COMPLETE] i2c_platform, mpu6050, ssd1306.
- **Connectivity Layer**: [COMPLETE] wifi_sta, mqtt_manager, connectivity_manager.
- **Service Layer**: 
    - Motion Monitor: [COMPLETE]
    - Display Service: [COMPLETE]
    - Fault Detector: [PENDING]
- **System Layer**:
    - System Controller: [PARTIAL]

## Next Session Focus
- Implement the **Fault Detector** service to analyze motion metrics for anomalies (shakes, freefall).
- Finalize the `system_controller` FSM transitions for error recovery.
- Add hardware-in-the-loop tests for the `display_service`.
