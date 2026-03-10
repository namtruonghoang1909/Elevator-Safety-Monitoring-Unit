# ESMU Testing Framework

This directory contains automated hardware-in-the-loop (HIL) tests for the Elevator Safety Monitoring Unit (ESMU) project, implemented using the Unity test framework and running directly on the ESP32.

## Test Structure

- **`test_connectivity/`**: Verifies WiFi station, MQTT connection, and Connectivity Manager orchestration.
- **`test_motion_monitor/`**: Verifies Exponential Moving Average (EMA) filtering, gravity compensation, and logical state transitions.
- **`test_mpu6050/`**: Verifies raw/scaled IMU data and register-level auto-recovery logic.
- **`test_ssd1306/`**: Verifies OLED page-writing, contrast control, and graphics boundaries.

## How to Run Tests

Open the **PlatformIO Core CLI** in your IDE and run the following commands:

### Run all tests:
```bash
pio test -e esp32doit-devkit-v1
```

### Run a specific test module (e.g., motion monitor):
```bash
pio test -e esp32doit-devkit-v1 -f test_motion_monitor
```

## Interactive Tests

Some tests (like `test_mpu6050`) use **GPIO 15** (built-in LED or external) to signal sampling phases.
- **Blinking**: Waiting for user action/stabilization.
- **OFF**: Active sampling phase.

## Important Notes

- **Hardware Required**: All tests in this directory are HIL tests and require a connected ESP32 with the relevant sensors (MPU6050, SSD1306).
- **Initialization**: `setUp()` functions typically initialize the I2C bus and relevant NVS partitions.
- **Credentials**: Connectivity tests rely on credentials defined in `include/system_config.h`.
