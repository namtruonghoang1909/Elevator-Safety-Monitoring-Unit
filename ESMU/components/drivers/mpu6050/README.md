# MPU6050 Driver

This component provides a robust driver for the MPU6050 6-axis accelerometer and gyroscope, optimized for the ESMU system.

## Features

- **Scalable Data Reading**: Supports both raw (ADC counts) and scaled (g, °/s) data retrieval.
- **Hardware-in-the-Loop Robustness**: Includes auto-recovery logic that detects and repairs register corruption (e.g., in `GYRO_CONFIG`).
- **Configurable FSR**: Supports all accelerometer (±2g to ±16g) and gyroscope (±250°/s to ±2000°/s) Full-Scale Ranges.
- **Integrated Calibration**: Support for baseline X/Y/Z offset calculations.
- **Temperature Sensing**: Built-in conversion for the internal temperature sensor.

## Hardware Connection (Default)

| MPU6050 Pin | ESP32 Pin | Function |
| :--- | :--- | :--- |
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SDA | GPIO 21 | I2C Data |
| SCL | GPIO 22 | I2C Clock |

## Public API

- `mpu6050_init(cfg, dev_id)`: Initialize and verify device existence and config.
- `mpu6050_read_raw(dev_id, data)`: Read 14 bytes of raw sensor data.
- `mpu6050_read_scaled(dev_id, data)`: Read data converted to physical units.
- `mpu6050_get_accel_magnitude(data)`: Calculate total acceleration vector length.
- `mpu6050_reset(dev_id)`: Trigger a hardware reset.

## Reliability Features

The driver performs a WHO_AM_I check during initialization and implements a `mpu6050_verify_registers` function that can be called periodically or after suspected failures to ensure the hardware state remains consistent with the software configuration.
