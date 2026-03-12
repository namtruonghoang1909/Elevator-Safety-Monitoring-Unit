# Elevator Safety Monitoring Unit (ESMU)

The Elevator Safety Monitoring Unit (ESMU) is a safety-critical IoT system designed to monitor elevator health and detect abnormal behaviors (excessive shakes, emergency stops, or abnormal speed changes).

## System Architecture
This project is structured as a monorepo containing:
- **`gateway-esp32/`**: The central supervisor based on ESP32 (ESP-IDF). Manages UI, connectivity (MQTT/WiFi), and distributed data.
- **`edge-stm32/`**: Peripheral node based on STM32 for real-time sensor processing (Pending).
- **`shared/`**: Common protocol definitions for Inter-MCU communication.

## Core Features
- **Motion Monitoring**: 6-axis IMU (MPU6050) with EMA filtering and state detection (Stationary, Moving UP/DOWN).
- **Interactive UI**: SSD1306 OLED display with a layered architecture, real-time elevator visualization, and custom art icons.
- **WiFi Provisioning**: Built-in SoftAP web portal for easy WiFi credential configuration.
- **Connectivity**: Robust MQTT integration for publishing elevator health and emergency alerts.
- **Hardware Integration**: Dedicated manual toggle for provisioning (GPIO 15) and status LED feedback (GPIO 4).

## Getting Started (Gateway)
### Prerequisites
- ESP-IDF Framework
- PlatformIO (VS Code Extension)

### Build and Flash
1. Navigate to the `gateway-esp32` directory.
2. Build: `pio run`
3. Flash: `pio run --target upload`

### WiFi Setup
1. On first boot, or by holding **GPIO 15 for 5 seconds**, the device enters **Config Mode**.
2. Connect to the WiFi network: `ESMU-Setup`.
3. Navigate to `http://192.168.4.1` in your browser.
4. Enter your local WiFi credentials and click **Connect**.

## Documentation
- [Agent Logs & Roadmap](./agent/)
- [Display Service Docs](./gateway-esp32/components/services/display/)
- [Connectivity Manager Docs](./gateway-esp32/components/services/connectivity/connectivity_manager/)
