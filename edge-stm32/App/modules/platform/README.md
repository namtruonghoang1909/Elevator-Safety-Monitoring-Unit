# ESMU Edge Platform Abstraction

The Edge Platform layer provides thread-safe abstractions for the low-level communication buses on the STM32. It encapsulates the STM32 HAL (Hardware Abstraction Layer) and integrates with FreeRTOS to provide a robust API for higher-level services.

## 🚌 Supported Buses

### 📡 `can_platform/`
- **Driver**: STM32 HAL CAN.
- **Protocol**: CAN 2.0B.
- **Baud Rate**: 500kbps.
- **Thread Safety**: Mutex-protected transmission and a non-blocking reception task.

### 🔌 `i2c_platform/`
- **Driver**: STM32 HAL I2C.
- **Devices**: MPU6050, SSD1306.
- **Speed**: 400kHz (Fast Mode).
- **Thread Safety**: Mutex-protected bus access for multiple sensors.

---
**Part of the Elevator Safety Monitoring Unit (ESMU).**
