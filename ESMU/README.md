# Elevator Safety Monitoring Unit (ESMU)

The ESMU is an ESP32-based system designed to monitor elevator safety by detecting abnormal motion, shakes, and emergency stops.

## Features

- **Motion Monitoring**: 6-axis accelerometer/gyroscope (MPU6050).
- **OLED Display**: Real-time status output (SSD1306).
- **Connectivity**: WiFi and MQTT for telemetry and emergency alerts.
- **Fail-Safe Design**: Centralized connectivity management and robust event handling.

## Development Environment

- **Framework**: ESP-IDF
- **Build System**: PlatformIO
- **Board**: ESP32 DoIt DevKit V1

## Getting Started

### Build and Flash

Open the **PlatformIO Core CLI** in your IDE and run:

1.  **Build Project**:
    ```bash
    pio run
    ```
2.  **Flash Firmware**:
    ```bash
    pio run --target upload
    ```
3.  **Monitor Output**:
    ```bash
    pio device monitor
    ```

### Running Tests

To execute unit tests on the hardware:

1.  **Run all tests**:
    ```bash
    pio test -e esp32doit-devkit-v1
    ```
2.  **Run a specific module (e.g., connectivity)**:
    ```bash
    pio test -e esp32doit-devkit-v1 -f test_connectivity
    ```

## Project Structure

- `components/`: Modular drivers and services (I2C, MPU6050, SSD1306, Connectivity).
- `src/`: Main application logic.
- `test/`: Unit tests for each component.
- `docs/`: Technical datasheets and pinout diagrams.
