# Elevator Safety Monitoring Unit (ESMU)

The **Elevator Safety Monitoring Unit (ESMU)** is a distributed, safety-critical IoT system designed for real-time elevator health monitoring and anomaly detection (Free Fall, Emergency Stops, and Excessive Vibration).

## 🏗️ System Architecture
The project is architected as a **Dual-Node Distributed System** connected via an industrial-grade **CAN 2.0A** bus:

- **[Gateway Node] ESP32 (ESP-IDF/FreeRTOS)**: Acts as the system supervisor. Manages high-level connectivity (WiFi/MQTT), the local OLED UI dashboard, and a SoftAP web portal for provisioning.
- **[Edge Node] STM32F103 (HAL/FreeRTOS)**: The real-time sensor processing hub. Interfaces with the MPU6050 IMU, performs high-frequency filtering (EMA), and executes fault detection logic.
- **[Shared Protocol] C-Headers**: A unified protocol definition (`esmu_protocol.h`) ensuring binary compatibility and structured frame exchange between the ESP32 and STM32.

## 🚀 Core Features
- **Distributed Sensing**: High-frequency (100Hz) IMU sampling and EMA (Exponential Moving Average) filtering on the STM32 Edge node.
- **Robust Communication**: Reliable inter-MCU data exchange over **CAN 2.0A** (500kbps) with custom ID priority (Emergency > Health > Heartbeat).
- **Interactive UI**: Layered SSD1306 OLED architecture on the Gateway, featuring real-time motion visualization, tilt-alerts, and status icons.
- **Smart Connectivity**:
  - **WiFi Provisioning**: Built-in SoftAP captive portal (`192.168.4.1`) for dynamic credential configuration.
  - **MQTT Telemetry**: Asynchronous publishing of elevator metrics and "Critical Alert" states to a Core IoT dashboard.
- **Industrial Stability**: Thread-safe **Board Support Packages (BSP)** using FreeRTOS Mutexes for shared bus (I2C/SPI/CAN) access.

## 🛠️ Hardware Stack
- **MCUs**: ESP32-WROOM-32, STM32F103C8T6 (BluePill).
- **Sensors**: MPU6050 (6-axis Accel/Gyro).
- **Display**: SSD1306, ST7789.
- **Communication**: MCP2551 CAN Transceivers.

## 📂 Project Structure
```text
├── gateway-esp32/     # ESP-IDF System Supervisor & UI
├── edge-stm32/        # STM32 HAL Real-time Sensor Node
├── shared/            # Common CAN Protocol & Data Structures
└── agent/             # AI-Assisted Development Logs & Project Roadmap
```

## 🚦 Getting Started
### Build Requirements
- **ESP32**: ESP-IDF v5.x or PlatformIO.
- **STM32**: STM32CubeIDE or `arm-none-eabi-gcc`.

### Quick Flash (Gateway)
1. Navigate to `gateway-esp32/`.
2. Build & Flash: `pio run --target upload`
3. **WiFi Setup**: Hold **GPIO 15 for 5 seconds** to enter Config Mode. Connect to `ESMU-Setup` and navigate to `http://192.168.4.1`.

## 📈 Roadmap & Documentation
The development of this project follows a structured **Research -> Strategy -> Execution** lifecycle, documented in the [Agent Context](./agent/context/roadmap.md).
- [CAN Protocol Specification](./shared/can_protocol/)
- [Motion Monitor Service](./gateway-esp32/components/services/motion_monitor/)
- [Display UI Architecture](./gateway-esp32/components/services/display/)
