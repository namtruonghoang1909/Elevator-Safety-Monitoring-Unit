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

---

## Current Project State
- **Drivers Layer**: [COMPLETE] i2c_platform, mpu6050, ssd1306.
- **Connectivity Layer**: [COMPLETE] wifi_sta, mqtt_manager, connectivity_manager.
- **Service Layer**: 
    - Display Service: [PENDING]
    - Motion Monitor: [PENDING]
    - Fault Detector: [PENDING]
- **System Layer**:
    - System Controller: [PARTIAL/DEFERRED]

## Next Session Focus
- Implement the `motion_monitor` service for high-pass filtering and gyro-zeroing.
- Implement the `display_service` for real-time visual output on the SSD1306.
- Finalize the `system_controller` FSM.
