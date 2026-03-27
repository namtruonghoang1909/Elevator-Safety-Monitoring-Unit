# Edge Telemetry Service

## Overview
The `edge_telemetry` service handles inter-node communication for the STM32 Edge node. It broadcasts real-time sensor metrics and node health status to the ESP32 Gateway via the CAN bus.

## Features
- **Periodic Health Broadcast (100ms)**: Sends `ele_health_t` with tilt, vibration magnitude, and overall health status (STABLE, WARNING, EMERGENCY).
- **Periodic Heartbeat (1000ms)**: Sends `edge_heartbeat_t` with node uptime and internal operational state.
- **Immediate Emergency Broadcast**: High-priority transmission of `ele_emergency_t` when critical faults (Free Fall, Impact) are detected.

## Integration
The service uses:
- `bsp_can` for low-level CAN transmission.
- `system_registry` as the source for real-time motion data.
- `shared/can_protocol` for unified packet definitions.

## APIs
```c
bool edge_telemetry_init(void);
void edge_telemetry_start(void);
void edge_telemetry_broadcast_emergency(fault_code_t fault, int16_t value);
```
