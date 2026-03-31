# ESMU Gateway BSP (Board Support Package)

The BSP provides thread-safe abstractions for the low-level communication buses on the ESP32. It ensures that multiple tasks (e.g., Motion Monitor, Display Service, CAN Manager) can share physical hardware resources without data corruption or bus collisions.

## 🚌 Supported Buses

### 📡 `can_bsp/`
- **Driver**: ESP-IDF TWAI (Two-Wire Automotive Interface).
- **Protocol**: CAN 2.0B (Standard).
- **Baud Rate**: 500kbps.
- **Thread Safety**: Mutex-protected transmission and a high-priority reception task.

### 🔌 `i2c_bsp/`
- **Driver**: ESP-IDF I2C Master.
- **Devices**: MPU6050, SSD1306.
- **Thread Safety**: Bus-level mutexes to prevent multi-tasking collisions.

### 📐 `spi_bsp/`
- **Driver**: ESP-IDF SPI Master.
- **Devices**: ST7789 Color TFT.
- **Performance**: High-speed DMA-backed transfers for smooth UI rendering.

---
**Part of the Elevator Safety Monitoring Unit (ESMU).**
