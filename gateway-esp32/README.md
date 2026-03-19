# Elevator Safety Monitoring Unit (ESMU)

The ESMU is an ESP32-based industrial monitoring system designed to enhance elevator safety. It provides real-time detection of abnormal motion, car tilt, and emergency stops, while providing field diagnostics via an OLED display and remote telemetry via MQTT.

## 🏗️ System Architecture

The project follows a layered service-oriented architecture:

### 1. Services Layer
- **[Motion Monitor](components/services/motion_monitor/README.md)**: EMA filtering, gravity compensation, and logical state tracking.
- **[Display Service](components/services/display/README.md)**: Multi-layered UI management for real-time visualization.
- **[Connectivity Stack](components/services/connectivity/README.md)**: Orchestrated WiFi and MQTT lifecycle management.

### 2. System Layer
- **[System Controller](components/system/README.md)**: Centralized FSM and event dispatching.
- **System Registry**: Thread-safe "Whiteboard" for cross-service telemetry.

### 3. Drivers & BSP
- **[MPU6050](components/drivers/mpu6050/README.md)**: Robust 6-axis IMU driver with auto-recovery logic.
- **[SSD1306](components/drivers/ssd1306/README.md)**: Optimized OLED display driver.
- **[I2C BSP](components/bsp/i2c_bsp/README.md)**: Thread-safe multi-bus I2C abstraction.

## 🚀 Getting Started

### Build and Flash
1. **Install PlatformIO** (CLI or IDE extension).
2. **Build**: `pio run`
3. **Flash**: `pio run --target upload --upload-port COM4`
4. **Monitor**: `pio device monitor --port COM4`

### Running Hardware-in-the-Loop Tests
Unit tests are written using the Unity framework and run directly on the ESP32:
```bash
# Run all tests
pio test -e esp32doit-devkit-v1

# Run a specific component test
pio test -e esp32doit-devkit-v1 -f test_motion_monitor
```

## 🛠️ Project Standards
- **Naming**: `snake_case_t` for types, `component_action` for functions.
- **Error Handling**: Explicit `esp_err_t` checks with `ESP_LOGE` (No silent `ESP_ERROR_CHECK`).
- **Documentation**: Doxygen-style comments in all public headers.
- **Thread Safety**: Centralized registry and per-service mutexes for concurrency.
