# ESMU Shared Resources

This directory contains resources shared between the **Edge Node (STM32)** and the **Gateway Node (ESP32)**. Its primary purpose is to ensure binary compatibility and a unified communication protocol across the distributed system.

## 📁 Contents

### 📡 `can_protocol/`
Contains the core definition of the ESMU inter-node protocol over CAN 2.0B.
- **`esmu_protocol.h`**: The main API for protocol handling and packet validation.
- **`protocol_packets.h`**: Definition of packet structures (Emergency, Health, Heartbeat).
- **`protocol_types.h`**: Base data types, node IDs, and message priority levels.

## 🚀 Key Benefits

- **Single Source of Truth**: Changes to packet structures or message IDs only need to be updated here.
- **Binary Compatibility**: Use of packed structs and standardized types ensures data is interpreted identically on both ARM (STM32) and Xtensa (ESP32) architectures.
- **Code Reuse**: Shared headers reduce duplication and potential for communication-breaking bugs.

---
**Part of the Elevator Safety Monitoring Unit (ESMU).**
