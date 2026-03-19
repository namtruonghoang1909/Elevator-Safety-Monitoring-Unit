# Task: Port MPU6050 and Motion Monitor to STM32

## Status: IN_PROGRESS

### 1. MPU6050 Driver Porting
- [ ] Copy `mpu6050.h` and `mpu6050.c` from ESP32 to `edge-stm32/App/modules/drivers/mpu6050/`.
- [ ] Adapt I2C calls from `i2c_master.h` (ESP32) to `bsp_i2c.h` (STM32).
- [ ] Update `edge-stm32/CMakeLists.txt` include paths.
- [ ] Initialize MPU6050 in `main.c` and verify "Who Am I" ID via `edge_logger`.

### 2. Motion Monitor Service Migration
- [ ] Copy `motion_monitor` logic to `edge-stm32/App/modules/services/motion_monitor/`.
- [ ] Adapt timing logic (replace `esp_timer_get_time` with `osKernelSysTick`).
- [ ] Integrate Motion Monitor into `StartDefaultTask`.

### 3. Real Data Transmission
- [ ] Replace STM32 mock data with real sensor readings from Motion Monitor.
- [ ] Verify real tilt/vibration data on ESP32 Serial Monitor.
