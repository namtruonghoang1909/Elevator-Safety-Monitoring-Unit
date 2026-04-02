# ESMU Gateway Platform Layer

The Platform Layer provides thread-safe abstractions for the low-level communication buses on the ESP32. It ensures that multiple tasks (e.g., Motion Monitor, Display Service, CAN Manager) can share physical hardware resources without data corruption or bus collisions.

> **Note**: This component was previously named "HAL". It has been renamed to "Platform" to better reflect its purpose as a Platform Abstraction Layer.

## 🚌 Supported Modules

### 📡 `can_platform/`
- **Driver**: ESP-IDF TWAI (Two-Wire Automotive Interface).
- **Naming**: `platform_can_<action>`
- **Thread Safety**: Mutex-protected transmission.

### 🔌 `i2c_platform/`
- **Driver**: ESP-IDF I2C Master (Modern Driver).
- **Naming**: `platform_i2c_<action>`
- **Thread Safety**: Bus-level management.

### 📐 `spi_platform/`
- **Driver**: ESP-IDF SPI Master.
- **Naming**: `platform_spi_<action>`
- **Performance**: DMA-backed transfers.

### 💡 `pwm_platform/`
- **Driver**: ESP-IDF LEDC.
- **Naming**: `platform_pwm_<action>`
- **Purpose**: Backlight and signal control.

### 📟 `uart_platform/`
- **Driver**: ESP-IDF UART.
- **Naming**: `platform_uart_<action>`
- **Features**: Background RX task with callbacks.

### 📍 `board_pins/`
- **Definition**: Project-wide GPIO mapping.
- **Purpose**: Centralized hardware configuration.

### 💾 `nvs_platform/`
- **Driver**: ESP-IDF NVS (Non-Volatile Storage).
- **Naming**: `platform_nvs_<action>`
- **Usage**: WiFi credentials and persistent settings.

---
**Part of the Elevator Safety Monitoring Unit (ESMU).**
