# ESMU Edge BSP (Board Support Package)

The Edge BSP provides thread-safe abstractions for the low-level communication buses on the STM32. It encapsulates the STM32 HAL (Hardware Abstraction Layer) and integrates with FreeRTOS to provide a robust API for higher-level services.

## 🚌 Supported Buses

### 📡 `bsp_can/`
- **Driver**: STM32 HAL CAN.
- **Protocol**: CAN 2.0B.
- **Baud Rate**: 500kbps.
- **Thread Safety**: Mutex-protected transmission and a non-blocking reception task.

### 🔌 `bsp_i2c/`
- **Driver**: STM32 HAL I2C.
- **Devices**: MPU6050.
- **Speed**: 400kHz (Fast Mode).
- **Thread Safety**: Mutex-protected bus access for multiple sensors.

---
**Part of the Elevator Safety Monitoring Unit (ESMU).**
