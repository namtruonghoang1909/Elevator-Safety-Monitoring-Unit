# ESMU Project Memory Log

...

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

### 26. Gateway HAL Refactoring & Standard Naming (April 1, 2026)
- **What**: Refactored the "BSP" layer into a proper "HAL" (Hardware Abstraction Layer) with standardized `hal_<module>_<action>` naming.
- **Where**: 
    - `gateway-esp32/components/bsp_hal/` (Renamed from `components/hal`)
    - All drivers (`st7789`, `ssd1306`, `sim_a7680c`)
    - All services (`motion_proxy`, `system`)
    - Test suite (`test_ssd1306`, `test_mqtt_protocol`)
- **Why**: To align the codebase with standard embedded architecture terminology where hardware-specific logic is abstracted by a HAL. The previous naming was inconsistent and caused directory name collisions with ESP-IDF internal components.
- **Fixes**:
    - **Naming**: Renamed all functions from `*_bsp_...` or `*_hal_...` to `hal_<module>_...` (e.g., `hal_i2c_write_bytes`).
    - **Filesystem**: Renamed `components/hal` to `components/bsp_hal` to resolve a conflict where ESP-IDF's `log` component required an internal `hal` component, causing build failures.
    - **Integration**: Updated all `CMakeLists.txt` files to point to the new `hal_*.c` source files and use the new component names.
    - **Consistency**: Updated all `#include` directives to use `*_hal.h`.
- **Result**: A clean, consistent, and collision-free hardware abstraction layer. Verified with a successful `pio run` build of the entire project.
