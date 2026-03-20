# Fixed Bugs Memory

### [2026-03-20] STM32 I2C Initialization Failure (Pre-Scheduler Context)
- **Problem**: `edge_logger_init` and `mpu6050_init` were failing when called from `main()` before `osKernelStart()`.
- **Cause**: `bsp_i2c` uses a FreeRTOS Mutex. Before the scheduler starts, `xSemaphoreTake` with a timeout either returns an error or behaves unpredictably because SysTick is not yet owned by the OS.
- **Solution**: Restored the `StartDefaultTask` in `main.c`. Moved all hardware and service initialization (System Registry, Logger, Motion Monitor) into the `StartDefaultTask`. This ensures the scheduler is running and Mutexes/Delays work correctly.

### [2026-03-20] FreeRTOS Task Creation Stuck / HardFault
- **Problem**: The program would hang immediately upon starting the `MotionTask`.
- **Cause**: 
    1. `configTOTAL_HEAP_SIZE` was only 3KB, which was too small for the allocated task stacks.
    2. `DefaultTask` stack was only 512 words, which was insufficient for the complex initialization logic.
- **Solution**: 
    1. Increased `configTOTAL_HEAP_SIZE` to 12KB in `FreeRTOSConfig.h`.
    2. Increased `DefaultTask` stack size to 1024 words in `main.c`.

### [2026-03-20] Linker Error: Undefined reference to `vTaskDelayUntil`
- **Problem**: Build failed during linking after refactoring to Native FreeRTOS.
- **Cause**: `INCLUDE_vTaskDelayUntil` was set to `0` in `FreeRTOSConfig.h`.
- **Solution**: Set `#define INCLUDE_vTaskDelayUntil 1` in `FreeRTOSConfig.h`.

### [2026-03-20] OLED Showing "TILT" during Shaking
- **Problem**: Physical vibration was being misidentified as static tilt.
- **Cause**: Tilt detection was using the Accelerometer, which picks up linear noise from vibrations.
- **Solution**: Refactored logic to decouple detections:
    - **Shake**: Now based strictly on Gyroscope magnitude (Angular velocity).
    - **Tilt**: Now based on Accelerometer magnitude, with a higher threshold and 3-axis offset calibration to "zero" the sensor to its mounted position.
