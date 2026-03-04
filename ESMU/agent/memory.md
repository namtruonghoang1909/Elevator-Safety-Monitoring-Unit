# ESMU Project Memory Log

## 2026-03-01 - Initial Drivers and Platform Implementation
- **i2c_platform**: Implemented project-level I2C abstraction. Supports multi-bus management and thread-safe device registration.
- **mpu6050**: Implemented driver for the 6-axis accelerometer/gyroscope.
- **ssd1306**: Implemented low-level OLED driver for 128x64 displays.

## 2026-03-03 - Build Fixes and Connectivity Stack Refactoring

### 1. Build System Resolution
- **Status**: SUCCESS.
- **Changes**:
    - Fixed `wifi_sta` dependencies by adding `esp_event` and `esp_netif` to `CMakeLists.txt`.
    - Fixed `mqtt_manager` dependencies by adding `esp_event` and `esp_common` to `CMakeLists.txt`.
    - Simplified root `CMakeLists.txt` for better component discovery.
    - Added `nvs_flash` dependency to the system component.

### 2. Connectivity Manager and Telemetry Integration
- **Status**: IMPLEMENTED & REFACTORED.
- **Changes**:
    - Refactored `src/main.c` to use `connectivity_manager` for centralized network control.
    - Implemented a `telemetry_task` that publishes JSON payloads (Uptime, Status, RSSI) to the broker every 10 seconds.
    - Verified logic for 5-second WiFi stability before starting MQTT.

### 3. System Controller Bug Fixes
- **Status**: PARTIAL (DEFERRED).
- **Changes**:
    - Added `system_controller_init()` to initialize the event queue.
    - Added safety checks to prevent `xQueueSend` on a NULL queue.
    - Deferred full implementation per user request to focus on the connectivity stack and drivers.

### 4. Unit Testing Framework & Bug Fixes (Task 05)
- **Status**: VERIFIED & FIXED.
- **Changes**:
    - Created `test/test_connectivity/test_connectivity.c` for automated testing.
    - **Bug Fix**: Corrected `connectivity_manager` state to `WIFI_ONLY` during stability wait.
    - **Bug Fix**: RSSI now reports -127 (no signal) when disconnected instead of 0.
    - **Bug Fix**: `connectivity_manager_stop` now correctly shuts down WiFi.
    - **Safety**: Added NULL pointer checks for WiFi credentials in `wifi_sta`.
    - Verified build and test structure using PlatformIO CLI in separate terminal windows.

---

## Project Structural State (Current)
- **Drivers Layer**: [COMPLETE] i2c_platform, mpu6050, ssd1306.
- **Connectivity Layer**: [COMPLETE & REFACTORED] wifi_sta, mqtt_manager, connectivity_manager.
- **Service Layer**: 
    - Display Service: [PENDING]
    - Fault Detector: [PENDING]
- **System Layer**:
    - System Controller: [PARTIAL/DEFERRED]

## 2026-03-03 - Final Connectivity Verification & Documentation
- **Status**: SUCCESS (All tests PASSED).
- **Changes**:
    - Re-implemented `test/test_connectivity/test_connectivity.c` to restore missing tests.
    - **Fix**: Made `wifi_sta_init` and `mqtt_manager_init` idempotent and safe for multiple calls.
    - **Test Improvement**: Added `tearDown` cleanup and adjusted RSSI assertions.
    - **Documentation**: Updated all READMEs to use standard `pio` commands for the PlatformIO Core CLI instead of absolute paths.
    - **File Management**: Renamed `test/instructions.md` back to `test/README.md` for consistency.
    - **Project Setup**: Created a root `README.md` with project overview and build/test instructions.

## Build Statistics (Latest)
- **Flash Usage**: 84.6% (887,521 bytes)
- **RAM Usage**: 10.5% (34,332 bytes)
- **Build Tool**: PlatformIO (Espressif32 6.12.0)
