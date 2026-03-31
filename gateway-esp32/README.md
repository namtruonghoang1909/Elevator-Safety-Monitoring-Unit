# ESMU Gateway Node (ESP32)

The **Gateway Node** acts as the central coordinator and communication bridge of the Elevator Safety Monitoring Unit (ESMU). It aggregates data from the Edge Node via the CAN bus, manages local visualization on a color TFT display, and handles cloud telemetry via WiFi and MQTT.

## 🚀 Key Responsibilities

- **System Coordination**: Maintaining the global system registry and health status for all nodes.
- **Inter-Node Communication**: Acting as a CAN 2.0B master, processing emergency and health messages from the Edge Node.
- **Cloud Telemetry**: Publishing real-time elevator metrics and fault alerts to the cloud via MQTT over WiFi.
- **Fail-Safe Alerts**: Providing a secondary cellular alert path using the SIM7600 module.
- **User Interface**: Managing high-resolution status dashboards on the ST7789 color display.

## 🏗️ Architecture

The firmware is developed using the **ESP-IDF v5.x** framework and follows a modular service-oriented architecture.

### Module Structure (`components/`)
- **`bsp/`**: Thread-safe board support packages for CAN, I2C, and SPI.
- **`drivers/`**: High-level drivers for SSD1306 (OLED), ST7789 (TFT), and SIM7600.
- **`services/`**: 
    - `communication`: CAN/MQTT message processing and protocol management.
    - `connectivity`: WiFi station management and auto-reconnect logic.
    - `motion_proxy`: Aggregating and translating Edge Node sensor data for telemetry.
- **`system/`**: Core boot sequences, system registry, and global state management.

## 🛠️ Technical Specifications

| Feature | Specification |
|---------|---------------|
| **MCU** | ESP32-WROOM-32 |
| **Framework** | ESP-IDF (FreeRTOS-based) |
| **Connectivity** | WiFi (802.11b/g/n), MQTT, SIM7600 (4G LTE) |
| **Display** | ST7789 (240x240 Color TFT via SPI) |
| **Bus Interface** | CAN 2.0B (TWAI), SPI (HSPI), I2C |

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
