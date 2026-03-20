# Task: Refactor to Native FreeRTOS and Implement MPU6050 Driver

## Status
- [x] MPU6050 Driver Verification & Initialization
- [x] Motion Monitor Service Migration
- [x] Refactor to Native FreeRTOS
- [ ] System Integration & Final Validation

## 1. Native FreeRTOS Refactor
- [x] Refactor `system_registry.c` to use `SemaphoreHandle_t`.
- [x] Refactor `motion_monitor.c` to use `xTaskCreate` and `vTaskDelay`.
- [x] Refactor `watchdog_service.c` to use `xTaskCreate`.
- [x] Refactor `system.c` to use `xTaskCreate`.
- [x] Refactor `main.c` to use `StartDefaultTask` with `xTaskCreate`.

## 2. Motion Monitor Service
- [x] Implement 100Hz sensing and filtering.
- [x] Implement 10ms registry updates.
- [x] Implement 500ms OLED logging ("H:GOOD/FAULT B:LEVEL/TILT/SHAKE").

## 3. Validation
- [ ] Verify OLED Logger shows initialization and calibration.
- [ ] Verify motion status updates on OLED in real-time.
