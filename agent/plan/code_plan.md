# Code Plan: Motion Monitor Refactoring (STM32 Edge Node)

## 1. Objective
Refactor the existing `motion_monitor.c` into a modular, testable service and solve the noise/fluctuation issues in Z (acceleration), V (vibration), and S (health score/displacement).

## 2. Structural Changes
Move from a single file to a modular directory structure under `edge-stm32/App/modules/services/motion_monitor/`:

```text
motion_monitor/
├── core/
│   ├── motion_core.c         (Task loop & system integration)
│   └── motion_core.h
├── kinematics/
│   ├── motion_kinematics.c   (Linear Z-accel & Speed estimation)
│   └── motion_kinematics.h
├── filters/
│   ├── motion_filters.c      (EMA & Deadband algorithms)
│   └── motion_filters.h
├── abnormal_detector/
│   ├── abnormal_detector.c   (Shake, FreeFall, Impact logic)
│   └── abnormal_detector.h
├── motion_monitor.c          (Public API Implementation)
└── motion_monitor.h          (Public API Header)
```

## 3. Algorithmic Improvements (Addressing Fluctuations)

### A. Improved Calibration
- Increase sample size (from 100 to 500 samples) to ensure a stable baseline.

### B. Noise Suppression (Filters & Deadbands)
- **Deadband Implementation**: Apply a small threshold (noise floor) to raw sensor data after calibration. If `|val| < threshold`, set `val = 0`. This suppresses the white noise of the MPU6050 when stationary.
- **Dynamic EMA**: Tunable alpha to balance responsiveness vs stability.

### C. Z, V, S Stability
- **Z (Linear Accel)**: Apply deadband filter to eliminate jitter in the acceleration vector at rest.
- **V (Vibration)**: Apply deadband filter to gyro magnitude to ensure V=0 when stationary.
- **S (Health Score)**: Derived from smoothed vibration (V), will stay at 100 when V is 0.

## 4. Implementation Steps
1. **Module Creation**: Define headers and move logic from `motion_monitor.c` into specialized folders.
2. **Filter Enhancement**: Add `motion_filter_deadband()` and `motion_filter_ema()`.
3. **Task Logic**: Update the main loop to call modular functions sequentially.
4. **Integration**: Update `system_registry` and `main.c` initialization to reflect the new structure.

## 5. Verification Plan
- **Stationary Test**: Confirm Z=0, V=0, S=100 (or stable values) when the board is flat and still.
- **Shake Test**: Verify fault detection triggers correctly.
