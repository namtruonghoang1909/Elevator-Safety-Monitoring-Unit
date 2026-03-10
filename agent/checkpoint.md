# Session Checkpoint - March 10, 2026 (Evening)

## Last Working Context
- **Architectural Shift**: Decided to move to a distributed **Edge/Gateway** architecture over **CAN Bus**.
    - **Edge Node (STM32)**: Inside elevator, handles MPU6050 sampling, `motion_monitor` logic, and safety-critical fault detection.
    - **Gateway Node (ESP32)**: Outside elevator, handles SSD1306 UI, WiFi/MQTT telemetry, Webserver for config, and SIM7600 (SMS/Calls).
- **Project Structure**: Proposed a multi-node root directory:
    - `gateway-esp32/` (Current ESP32 code)
    - `edge-stm32/` (New STM32 project)
    - `shared/protocol/` (Shared `can_protocol.h` for communication "contract")
- **Display Service**: Finalized UI enhancements, including the layered architecture, dynamic elevator tilt visualization, and status bar icons.
- **VS Code Config**: Discussed removing hardcoded absolute paths from `.vscode/settings.json` and using `${workspaceFolder}` or multi-root workspace files.

## Project State
- **Drivers Layer**: Verified (MPU6050, SSD1306, I2C Platform).
- **Connectivity Layer**: Verified (WiFi, MQTT, Connectivity Manager).
- **Service Layer**:
    - **Motion Monitor**: COMPLETE (To be ported to STM32 Edge).
    - **Display Service**: COMPLETE (Gateway UI finalized).
    - **Fault Detector**: PENDING (To be implemented on STM32 Edge).
- **System Layer**:
    - **System Controller**: PARTIAL (FSM logic needs finalization).

## Next Recommended Steps
1. **Refactor Root**: Move current project files into a `gateway-esp32/` subdirectory.
2. **Define Protocol**: Create `shared/protocol/can_protocol.h` to define the CAN message structure (Heartbeats, Emergency Frames).
3. **Initialize Edge**: Setup the `edge-stm32/` project and port `mpu6050` and `motion_monitor` logic.
4. **Implement CAN Drivers**: Add CAN bus support to both nodes to bridge the hardware gap.
5. **Webserver Implementation**: Develop the configuration and monitoring web dashboard on the Gateway (ESP32).
