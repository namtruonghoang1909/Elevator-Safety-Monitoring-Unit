# Task: Fix STM32 Edge Node Build Errors

## Goal
Resolve compilation errors and warnings in the `edge-stm32` project to achieve a successful build.

## Todo List
- [x] Fix `telemetry_task` undeclared error in `edge_telemetry.c`.
- [x] Resolve `send_heartbeat_packet` unused function warning in `edge_telemetry.c` by implementing `telemetry_task`.
- [x] Resolve `filt_x` and `filt_y` unused variable warnings in `motion_monitor.c`.
- [x] Fix `avg_tilt` undeclared error and too few arguments in `system.c` by removing redundant emergency broadcast call.
- [x] Verify build success for `edge-stm32` (Fixed code, waiting for build confirmation).

# Task: Implement Remote Fault Acknowledgment

## Goal
Implement a mechanism to acknowledge and clear faults from a remote CoreIoT dashboard via MQTT.

## Todo List
- [ ] **Design**: Define MQTT command topic and message format (e.g., `esmu/cmd/ack`).
- [ ] **Gateway (ESP32)**:
    - [ ] Update `mqtt_manager` to subscribe to the command topic.
    - [ ] Implement `system_registry_clear_fault()` to reset `fault_active` and `current_state`.
    - [ ] Handle incoming ACK messages in the MQTT callback.
    - [ ] Publish a "Fault Cleared" status (`fault_code = 0`) back to MQTT after reset.
- [ ] **Edge (STM32)**:
    - [ ] Check if the Edge node latches its emergency state and if it needs a "Reset" CAN message from the Gateway.
- [ ] **Verification**: Test the end-to-end flow using MQTT explorer or a simulation script.
