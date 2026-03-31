# ESMU Edge Services

This directory contains the core real-time logic for the ESMU Edge node. These services prioritize deterministic performance and low-latency fault detection.

## 🚀 Services

### 📊 `motion_monitor/`
The primary data processing service for IMU data.
- **IMU Sampler**: Triggers MPU6050 acquisition at a steady 100Hz.
- **DSP Engine**: Implements Exponential Moving Average (EMA) filters for noise reduction.
- **Fault Detection**: Implements algorithms for:
    - **Free Fall**: Detection of near-zero gravity.
    - **Impact**: High-G acceleration detection.
    - **Vibration**: Gyro-based frequency analysis.

### 📝 `edge_logger/`
A local diagnostic logging service.
- **Worst-State Capture**: Records the highest vibration or tilt value within a time window for field review.
- **UI Interface**: (Optional) Manages a local debugging OLED for on-site technicians.

### 🐕 `watchdog/`
Ensures system reliability in high-interference environments.
- **Task Monitor**: Periodically checks the health of all running FreeRTOS tasks.
- **Auto-Recovery**: Triggers a system reset if critical tasks hang or the MPU6050 communication fails.

---
**Part of the Elevator Safety Monitoring Unit (ESMU).**
