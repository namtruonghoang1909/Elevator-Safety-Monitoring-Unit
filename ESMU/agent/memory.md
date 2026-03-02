# ESMU Project Memory Log

## 2026-03-01 - Initial Drivers and Platform Implementation
- **i2c_platform**: Implemented project-level I2C abstraction. Supports multi-bus management and thread-safe device registration.
- **mpu6050**: Implemented driver for the 6-axis accelerometer/gyroscope.
- **ssd1306**: Implemented low-level OLED driver for 128x64 displays.

## 2026-03-02 - Connectivity Stack Implementation

### 1. WiFi Station Component (`wifi_sta`)
- **Status**: Implemented & Verified.
- **Features**: 
    - Clean wrapper for ESP-IDF WiFi APIs.
    - Thread-safe connection status checks.
    - Background health monitoring task (logs SSID, RSSI, and State every 5 seconds).
    - RSSI-to-Strength mapping (Strong, Moderate, Weak, Very Weak).

### 2. MQTT Manager Component (`mqtt_manager`)
- **Status**: Implemented & Verified.
- **Features**:
    - Wrapper for `esp_mqtt_client`.
    - Supports explicit Broker URI, Port, and Authentication (Username/Password).
    - Thread-safe `publish` and `subscribe` APIs.
    - Internal event handling for connectivity and data reception.

### 3. Connectivity Manager (`connectivity_manager`)
- **Status**: Implemented & Verified.
- **Architectural Goal**: Orchestrate the WiFi and MQTT lifecycles to handle unstable network environments.
- **Key Logic**:
    - **Stability Guard**: Waits for 5 seconds of continuous WiFi stability before attempting to start MQTT.
    - **Automatic Lifecycle**: Stops MQTT immediately if WiFi is lost; restarts MQTT once WiFi is stable again.
    - **RSSI Watchdog**: Warns the system if signal strength drops below -90dBm.
    - **Simplified API**: Provides `connectivity_manager_is_ready()` as a single-point check for high-level tasks.

### 4. Entry Point Refactoring (`main.c`)
- **Status**: Integrated.
- **Features**:
    - Uses `connectivity_manager` for centralized network control.
    - Implements a `telemetry_task` that publishes JSON payloads (Uptime, Status, RSSI) to the broker every 10 seconds, but only when `connectivity_manager_is_ready()` returns true.

---

## Project Structural State (Current)
- **Drivers Layer**: [COMPLETE] i2c_platform, mpu6050, ssd1306.
- **Connectivity Layer**: [COMPLETE] wifi_sta, mqtt_manager, connectivity_manager.
- **Service Layer**: 
    - Display Service: [PENDING]
    - Fault Detector: [PENDING]
- **System Layer**:
    - System Controller: [PENDING]

## Build Statistics (Latest)
- **Flash Usage**: 84.8% (889,065 bytes)
- **RAM Usage**: 10.5% (34,312 bytes)
- **Build Tool**: PlatformIO (Espressif32 6.12.0)
