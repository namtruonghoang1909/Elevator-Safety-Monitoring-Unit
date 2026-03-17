# Node Communication Protocol & CAN Platform Design

## Objective
Establish a shared communication protocol between the ESP32 Gateway and STM32 Edge nodes using the CAN bus, and implement a reusable `can_platform` abstraction for the Gateway.

## Branch Strategy
- **Branch Name**: `feature/can-platform`
- **Scope**:
    - **Step 1: Shared Protocol Definition** (Root `shared/protocol/`)
        - `protocol_types.h`: Basic enums (states, fault codes, balance states).
        - `protocol_packets.h`: Packed data structures for CAN payloads.
        - `protocol.h`: Main header for API and ID definitions.
    - **Step 2: CAN Platform Implementation** (Gateway `components/platform/can_platform/`)
        - Abstraction for ESP32 TWAI APIs (similar to `i2c_platform`).

## 1. Protocol Definition

### CAN Message IDs
| Priority | ID (Hex) | Macro Name | Description | Frequency |
| :--- | :--- | :--- | :--- | :--- |
| **Critical** | `0x010` | `CAN_ID_ELE_EMERGENCY` | Immediate fault alert. | Interrupt |
| **High** | `0x100` | `CAN_ID_ELE_HEALTH` | Real-time motion/balance data. | 100ms |
| **Medium** | `0x200` | `CAN_ID_EDGE_HEALTH` | Edge node health/uptime. | 1000ms |

### Data Structures (8-byte Packed)

#### A. Elevator Health (`CAN_ID_ELE_HEALTH`)
*Focus: Real-time status for Display and Telemetry aggregation.*
- `int16_t avg_tilt`: Average tilt magnitude (scaled).
- `int16_t max_tilt`: Peak tilt seen in last window.
- `uint8_t balance_state`: Enum (Level, Tilt Left, etc.).
- `uint8_t health_score`: 0-100 score calculated by Edge.
- *Reserved/Padding*: 2 bytes.

#### B. Elevator Emergency (`CAN_ID_ELE_EMERGENCY`)
*Focus: Immediate alert with diagnostic data.*
- `uint8_t fault_code`: Enum (SHAKE, STOP, FALL).
- `uint8_t severity`: 1-5.
- `int16_t fault_value`: The sensor value that triggered the fault (e.g., the 5.0g peak).
- *Reserved/Padding*: 4 bytes.

#### C. Edge Health (`CAN_ID_EDGE_HEALTH`)
*Focus: Node heartbeat and diagnostics.*
- `uint8_t edge_health`: Enum (OK, SENSOR_ERROR, CAN_ERROR).
- `uint32_t uptime_sec`: Node uptime since boot.
- *Reserved/Padding*: 3 bytes.

## 2. CAN Platform Abstraction (`can_platform`)

### Architectural Strategy
- Based on ESP-IDF **TWAI (Two-Wire Automotive Interface)** driver.
- Location: `gateway-esp32/components/platform/can_platform`.
- Provides a thread-safe API for bus initialization and frame exchange.

### Key Functional Ideas
- **Bus Config**: Supports standard baud rates (500kbps default).
- **Mode Switching**: Supports NORMAL and LOOPBACK (for testing without nodes).
- **Frame Handling**: Abstracts the `twai_message_t` into a simpler `id`, `data`, `len` structure for the service layer.
- **Resource Protection**: Uses internal state management to prevent multiple initializations.

## 3. Implementation Steps
1. **Define Protocol**: Create the 3-file structure in `shared/protocol/`.
2. **Setup Gateway Includes**: Modify ESP32 build system to recognize the `shared/` folder.
3. **Setup Edge Includes**: Modify STM32 build system to recognize the `shared/` folder.
4. **Implement CAN Platform**: Create the TWAI abstraction on the ESP32.

## 4. Verification Plan
- **Struct Alignment**: Verify `sizeof()` matches 8 bytes on both GCC (ESP32) and ARM-GCC (STM32).
- **Loopback Test**: Send and receive protocol packets using `can_platform` in loopback mode.
