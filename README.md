# Elevator Safety Monitoring Unit (ESMU)

[![Status](https://img.shields.io/badge/Status-Complete-success.svg)](#)
[![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20STM32-blue.svg)](#)
[![Protocol](https://img.shields.io/badge/Protocol-CAN%202.0B-orange.svg)](#)

The **ESMU** is a high-reliability, distributed safety-critical system designed for real-time elevator health monitoring and fault detection. By separating high-frequency sensor fusion from cloud connectivity, the system ensures deterministic safety responses while providing rich telemetry via MQTT and a local color UI.

## 🏗️ System Architecture

The ESMU utilizes a **Dual-Node Distributed Architecture** connected via an industrial-grade **CAN 2.0B** bus.

### 1. Edge Node (STM32F103)
*The "Brain"*
- **Role**: High-speed data acquisition and real-time fault detection.
- **Hardware**: STM32F103 BluePill + MPU6050 (6-axis Accel/Gyro).
- **Functions**: 
    - 100Hz motion sampling with EMA (Exponential Moving Average) filtering.
    - Fault detection algorithms (Free Fall, Sudden Impact, Overtilt, High-frequency vibration).
    - 50ms debounced emergency state management.
- **OS**: Native FreeRTOS.

### 2. Gateway Node (ESP32)
*The "Communicator"*
- **Role**: Data aggregation, local visualization, and cloud telemetry.
- **Hardware**: ESP32-WROOM + ST7789 (240x240 Color TFT) + SIM7600 (Cellular Backup).
- **Functions**:
    - CAN-to-MQTT proxy for remote monitoring.
    - WiFi Station mode with automatic failover to SIM7600.
    - Local system registry for health tracking of all nodes.
    - Rich UI for real-time status and diagnostic logging.
- **OS**: ESP-IDF / FreeRTOS.

## 🚀 Key Features

- **Distributed Determinism**: Safety-critical logic runs on the STM32 Edge node, isolated from network-induced latency on the ESP32.
- **Smart Fault Detection**: Implements hysteresis and EMA filtering to prevent false positives while maintaining 100ms response times for actual emergencies.
- **Priority-Based CAN Protocol**:
    - **EMERGENCY (Highest)**: Immediate interrupt for detected faults.
    - **HEALTH (High)**: 100ms periodic motion/balance metrics.
    - **HEARTBEAT (Medium)**: 1s node status and uptime reporting.
- **Fail-Safe Connectivity**: Dual-path telemetry (WiFi + Cellular) ensures alerts reach the cloud even if building internet fails.
- **Diagnostic UI**: Local color display provides real-time "Worst-Case" logging and system-wide status at a glance.

## 🛠️ Technical Stack

| Component | Technology |
|-----------|------------|
| **Core Frameworks** | ESP-IDF (ESP32), STM32 HAL (STM32) |
| **Real-Time OS** | FreeRTOS (Native on both nodes) |
| **Communication** | CAN 2.0B (Standard), MQTT, SPI (ST7789), I2C (MPU6050) |
| **Build Tools** | PlatformIO (Gateway), STM32CubeIDE / Makefile (Edge) |
| **Language** | C (Embedded) |

## 📂 Repository Structure

```text
├── agent/                # AI Agent context, rules, and project roadmap
├── edge-stm32/           # STM32 Edge Node firmware (MPU6050 + CAN)
├── gateway-esp32/        # ESP32 Gateway Node firmware (WiFi + MQTT + UI)
├── shared/               # Shared CAN protocol headers and types
└── docs/                 # Data sheets, pinouts, and register maps
```

## 🚥 Safety & Fault Logic

The system monitors four primary fault conditions:
1. **Free Fall**: Detection of acceleration below 0.35g.
2. **Sudden Impact**: Detection of acceleration exceeding 1.6g.
3. **Vibration Analysis**: Gyro-based analysis categorized into `LOW`, `WARN`, and `CRITICAL`.
4. **Emergency Stop**: Rapid deceleration detection filtered through a 5-cycle debounce window.

