# ESMU Edge Node (STM32)

The **Edge Node** is the real-time sensor processing hub of the Elevator Safety Monitoring Unit (ESMU). It is responsible for high-frequency data acquisition from the MPU6050 IMU, performing digital signal processing, and executing safety-critical fault detection logic.

## 🚀 Key Responsibilities

- **Real-Time Sampling**: 100Hz acquisition of 6-axis accelerometer and gyroscope data.
- **Signal Processing**: Exponential Moving Average (EMA) filtering for noise reduction and gravity compensation.
- **Fault Detection**: Identifying Free Fall, Sudden Impacts, and abnormal vibration patterns.
- **State Management**: Managing system health and emergency states with robust debouncing.
- **Communication**: Reporting status and alerts to the Gateway via the CAN bus.

## 🏗️ Architecture

The firmware is built using the **STM32 HAL** and runs on **Native FreeRTOS**.

### Module Structure (`App/modules/`)
- **`bsp/`**: Board Support Package providing thread-safe abstractions for CAN and I2C buses.
- **`drivers/`**: Hardware-specific drivers (e.g., MPU6050).
- **`services/`**: High-level logic including `motion_monitor` and `edge_logger`.
- **`system/`**: Global system state, heartbeat, and registry management.

## 🛠️ Technical Specifications

| Feature | Specification |
|---------|---------------|
| **MCU** | STM32F103C8T6 (BluePill) |
| **Clock** | 72 MHz |
| **RTOS** | FreeRTOS (Native API) |
| **Sensing** | MPU6050 (I2C @ 400kHz) |
| **Bus** | CAN 2.0B (500 kbps) |

## 🚦 Safety Logic Thresholds

| Fault | Condition | Debounce |
|-------|-----------|----------|
| **Free Fall** | Total Accel < 0.35g | Immediate |
| **Sudden Impact** | Total Accel > 1.6g | Immediate |
| **High Vibration** | Gyro > 15.0 deg/s | 50ms (5 cycles) |

## 🔧 Development

### Build System
The project uses **STM32CubeIDE** for project management and the **ARM GCC Toolchain** for compilation.

### Flashing
Use an **ST-Link V2** or compatible debugger.
```bash
# Using STM32CubeProgrammer CLI
STM32_Programmer_CLI -c port=SWD -w build/edge-stm32.bin 0x08000000 -v -rst
```

---
**Part of the Elevator Safety Monitoring Unit (ESMU).**
