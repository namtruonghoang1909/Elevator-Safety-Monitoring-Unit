# ESMU CAN Protocol

The **ESMU CAN Protocol** is the unified communication layer between the Edge and Gateway nodes, built on **Standard CAN 2.0B** at 500kbps. It is designed for maximum reliability and deterministic handling of safety-critical data.

## 📡 Message Priority & IDs

The protocol uses an 11-bit standard identifier, with lower numerical values indicating higher priority.

| Message Type | Priority | ID Range | Description |
|--------------|----------|----------|-------------|
| **EMERGENCY**| Highest | `0x100 - 0x1FF` | Immediate alerts for detected faults (Free Fall, Impact). |
| **HEALTH**   | High    | `0x200 - 0x2FF` | Periodic (100ms) motion and sensor metrics. |
| **HEARTBEAT**| Medium  | `0x300 - 0x3FF` | Periodic (1s) node status and uptime. |

## 🏗️ Protocol Components

- **`protocol_types.h`**: Defines the base types, enums for node IDs (`NODE_ID_EDGE`, `NODE_ID_GATEWAY`), and fault codes.
- **`protocol_packets.h`**: Defines the structure of the data payloads. All structures are marked with `__attribute__((packed))` to ensure binary alignment across architectures.
- **`esmu_protocol.h`**: Provides high-level helper functions for frame construction and packet decoding.

## 🚀 Key Data Structures

### `emergency_packet_t`
Contains detailed information about a detected fault.
- **`fault_type`**: The specific error (FREE_FALL, OVERTILT, etc.).
- **`timestamp`**: Relative time of the event on the Edge node.
- **`raw_value`**: The value that triggered the fault.

### `health_packet_t`
Contains real-time sensor data.
- **`accel_x/y/z`**: Filtered acceleration in g-force.
- **`gyro_x/y/z`**: Filtered rotation rate in deg/s.

---
**Part of the Elevator Safety Monitoring Unit (ESMU).**
