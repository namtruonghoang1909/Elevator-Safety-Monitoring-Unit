# ESMU Gateway Node (ESP32)

The **Gateway Node** acts as the central coordinator and communication bridge of the Elevator Safety Monitoring Unit (ESMU). It aggregates data from the Edge Node via the CAN bus, manages local visualization on an OLED/TFT display, and handles cloud telemetry via WiFi and MQTT.

## 🚀 Key Responsibilities

- **System Coordination**: Maintaining the global system registry and health status for all nodes.
- **Inter-Node Communication**: Acting as a CAN 2.0B master, processing emergency and health messages from the Edge Node.
- **Cloud Telemetry**: Publishing real-time elevator metrics and fault alerts to the cloud via MQTT over WiFi.
- **Fail-Safe Alerts**: Providing a secondary cellular alert path using the `cellular_service` (A7680C 4G Module).
- **User Interface**: Managing high-resolution status dashboards on SSD1306/ST7789 displays.

## 🏗️ Architecture

The firmware is developed using the **ESP-IDF v5.x** framework and follows a modular service-oriented architecture.

### Module Structure (`components/`)
- **`bsp/`**: Thread-safe board support packages for CAN, I2C, SPI, UART, and PWM.
- **`drivers/`**: High-level drivers for SSD1306 (OLED), ST7789 (TFT), and A7680C (4G).
- **`services/`**: 
    - `communication`: `cellular_service` (4G FSM), `telemetry_service` (MQTT).
    - `connectivity`: `wifi_manager`, `mqtt_manager`, `connectivity_manager`.
    - `motion_proxy`: Aggregating and translating Edge Node sensor data for telemetry.
- **`system/`**: Core boot sequences (`system_hw`), system registry, and global state management.

## 🛠️ Technical Specifications

| Feature | Specification |
|---------|---------------|
| **MCU** | ESP32-WROOM-32 |
| **Framework** | ESP-IDF (FreeRTOS-based) |
| **Connectivity** | WiFi (802.11b/g/n), MQTT, A7680C (4G LTE) |
| **Display** | SSD1306 (OLED via I2C), ST7789 (TFT via SPI) |
| **Bus Interface** | CAN 2.0B (TWAI), UART |

## 🔧 Development

### Build System
The project uses **PlatformIO** for dependency management and build orchestration.

### Flashing
Ensure the ESP32 is connected via USB-Serial.
```bash
# Build and Flash using PlatformIO CLI
pio run --target upload
```

### Testing
Unit and integration tests are located in the `test/` directory and use the Unity framework.
```bash
# Run all hardware tests
pio test -e esp32doit-devkit-v1
```

---
**Part of the Elevator Safety Monitoring Unit (ESMU).**
