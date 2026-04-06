# Bug Fixing Plan - Display Switching and State Instability

## Issue Description
The user reports that the Gateway display is switching between "Standby Mode" and "Monitoring Display" (metrics) back and forth. This suggests that the `edge_armed` flag on the Gateway (derived from the STM32's `edge_state`) is toggling. Additionally, intermittent faults can cause the emergency overlay to trigger and re-trigger rapidly due to `fault_active` being cleared by periodic health packets and set by emergency packets.

## Root Cause Analysis
1.  **Watchdog Timeout**: The ESP32 watchdog for heartbeats is set to 3s (`CAN_TIMEOUT_MS_EDGE_OFFLINE`). If heartbeats (sent every 1s) are delayed due to CAN bus congestion or processing overhead, the watchdog might trigger `edge_armed = false`, then a heartbeat arrives and sets it back to `true`.
2.  **Fault Toggling**: In `system_registry.c` (ESP32), `system_registry_update_from_protocol_health` clears `fault_active` immediately if `HEALTH_STABLE` is received. If an `EMERGENCY` packet was just received, the next 1s health packet will clear the flag unless the fault is still actively detected as `HEALTH_EMERGENCY` on the STM32.
3.  **Display Refresh**: The `display_task` re-triggers the 5s emergency overlay whenever it sees a `false -> true` transition of `fault_active`. If the flag toggles, the overlay triggers repeatedly.
4.  **STM32 Debounce**: The STM32's `abnormal_detector` has a 100ms debounce (10 cycles). This might be too short for intermittent vibration spikes, leading to rapid toggling of the health state.

## Proposed Fixes
### 1. Increase ESP32 Watchdog Timeout
Modify `shared/can_protocol/esmu_protocol.h` to increase `CAN_TIMEOUT_MS_EDGE_OFFLINE` to 5000ms.

### 2. Stabilize ESP32 Emergency Logic
Modify `gateway-esp32/app/display/core/display.c` to ensure the emergency overlay doesn't re-trigger if it was just shown or if the fault is persistent. Add a cooldown or a state check.

### 3. Improve STM32 Fault Latching
Increase `DEBOUNCE_CYCLES` in `edge-stm32/App/modules/services/motion_monitor/abnormal_detector/abnormal_detector.c` to 20 (200ms) for better stability against transients.

### 4. Optimize Dashboard Redraw
Modify `gateway-esp32/app/display/ui/dashboard.c` to only clear and redraw the parts of the metrics area that actually changed, or at least avoid a full fill if only uptime changed.

## Todo List
- [ ] **Protocol**: Modify `shared/can_protocol/esmu_protocol.h` (`CAN_TIMEOUT_MS_EDGE_OFFLINE` -> 5000)
- [ ] **STM32**: Modify `edge-stm32/App/modules/services/motion_monitor/abnormal_detector/abnormal_detector.c` (`DEBOUNCE_CYCLES` -> 20)
- [ ] **ESP32 Display**: Refactor `display_task` in `gateway-esp32/app/display/core/display.c` to prevent rapid re-triggering.
- [ ] **ESP32 Registry**: Update `system_registry_update_from_protocol_health` to not clear `fault_active` immediately if the system is in `SYSTEM_STATE_ERROR`.
- [ ] **Verification**: Rebuild both nodes and verify stability.
