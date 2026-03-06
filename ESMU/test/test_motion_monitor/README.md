# Motion Monitor Unit Test Suite

This test suite verifies the internal logic, state machine, and mathematical processing of the `motion_monitor` service. 

## Key Features
- **Data Injection**: Uses the `motion_monitor_inject_sample` API to feed synthetic data into the logic without requiring physical hardware movement.
- **TEST Mode**: Initializes the service with `mpu_dev_id = 0xFF`, which disables the background polling task, ensuring deterministic results during injection tests.
- **Debounce Verification**: Confirms that the state machine correctly ignores short noise pulses and only transitions on sustained acceleration.

## Test Scenarios
1. **Transition Moving Up**: Verifies `STATIONARY` -> `MOVING_UP` -> `STATIONARY` cycle.
2. **Transition Moving Down**: Verifies `STATIONARY` -> `MOVING_DOWN` -> `STATIONARY` cycle.
3. **Transition Accelerating**: Verifies detection of high-G pulses (>0.15g).
4. **Debounce Rejection**: Proves that pulses shorter than 200ms are ignored.
5. **Shake Magnitude**: Validates the Euclidean norm calculation for XY vibration.

## How to Run
Ensure your ESP32 is connected, then run:
```bash
pio test -e esp32doit-devkit-v1 -f test_motion_monitor
```
