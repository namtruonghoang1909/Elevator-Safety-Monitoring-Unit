# MPU6050 Driver Unit Tests

This directory contains hardware-in-the-loop (HIL) unit tests for the MPU6050 6-axis motion tracking driver. These tests are interactive and use a physical LED signal to guide the user.

## Interaction Protocol

A Status LED (Default: **GPIO 15**) is used to communicate the test state without needing to look at the terminal:

1.  **Rapid Blinking (4Hz) for 5 seconds**: **WAITING**. Interaction is needed (place board flat or flip it).
2.  **LED OFF**: **SAMPLING**. Stay still! The driver is collecting and verifying data samples.

## What is Tested?

The tests verify the following functionality:

1.  **Communication & Identity (`test_mpu6050_communication`)**:
    *   Scans the I2C bus to confirm the device is reachable.
    *   Verifies `mpu6050_init` and `mpu6050_is_present` by reading the `WHO_AM_I` register.
2.  **Scaling & Full-Scale Range (`test_mpu6050_scaling_and_fsr`)**:
    *   **Interactive**: LED blinks for 5s; place the board **FLAT**.
    *   **2G Verification**: LED turns OFF; takes 5 samples (1s intervals) and verifies raw Z-axis is ~16384 LSB.
    *   **16G Verification**: Automatically re-configures and verifies raw Z-axis is ~2048 LSB.
3.  **Physical Vector Validation (`test_mpu6050_physical_vectors`)**:
    *   **Initial Check**: Verifies the stationary magnitude is 1.0g (±0.1g) while flat.
    *   **Interactive**: LED blinks for 5s; **FLIP** the board upside down.
    *   **Upside-Down Verification**: LED turns OFF; takes 5 samples (1s intervals) and verifies the Z-axis vector has flipped to negative (<-0.7g).
4.  **Robustness & Auto-Recovery (`test_mpu6050_robustness`)**:
    *   **Fault Tolerance**: Manually corrupts the `GYRO_CONFIG` register using raw I2C writes.
    *   **Self-Healing**: Verifies that the driver's `verify_registers` logic (via `init`) detects the corruption and restores the correct register value (0x00).

## How to Run the Tests

These tests must run on the target ESP32 hardware with an MPU6050 sensor connected via I2C (Default: SDA=21, SCL=22) and an LED on GPIO 15.

### Using PlatformIO Core CLI

Open the CLI in your IDE and run:

```bash
pio test -e esp32doit-devkit-v1 -f test_mpu6050
```

## Implementation Details

*   **Framework**: Unity Test Framework.
*   **Tolerance**: Uses a 0.1g tolerance for magnitude checks to accommodate sensor noise, tilt, and lack of factory calibration.
*   **Idempotency**: Tests remove the I2C device in `tearDown()` to ensure a clean slate and prevent ID leaks.
