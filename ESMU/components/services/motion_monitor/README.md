# Motion Monitor Service

The `motion_monitor` service is the "Inner Ear" of the Elevator Safety Monitoring Unit (ESMU). It processes raw high-frequency data from the MPU6050 sensor to provide stable, filtered metrics and logical elevator states.

## Key Features

- **Noise Reduction**: Uses an Exponential Moving Average (EMA) filter to eliminate motor and rail jitter.
- **Gravity Isolation**: Calibration logic subtracts the constant 1.0g gravity vector to isolate dynamic vertical movement (Linear Z).
- **Vibration Analysis**: Calculates the Euclidean magnitude of horizontal (XY) forces to detect ride quality issues.
- **State Machine**: A debounced logical engine that identifies elevator phases:
    - `STATIONARY`: The car is still.
    - `MOVING_UP/DOWN`: Constant speed or acceleration detected.
    - `DECELERATING_UP/DOWN`: Braking pulse detected at the end of a trip.
    - `ACCELERATING`: Generic high-G transient detection.

## Public API

```c
// Initialize the service and background task
esp_err_t motion_monitor_init(const motion_monitor_config_t *cfg);

// Retrieve latest filtered metrics (thread-safe)
esp_err_t motion_monitor_get_metrics(motion_metrics_t *out);

// Get current high-level state
motion_state_t motion_monitor_get_state(void);

// Perform blocking 2-second calibration (Must be stationary)
esp_err_t motion_monitor_calibrate(void);
```

## Testing & Verification

### Unit Tests
The component includes a Unity test suite that uses **Data Injection** to verify the state machine logic without requiring physical movement.
- **Location**: `test/test_motion_monitor/`
- **Command**: `pio test -e esp32doit-devkit-v1 -f test_motion_monitor`

### Integration Test
A detailed hardware-in-the-loop test is provided in `src/main.c` which logs raw vs. calculated data at 10Hz.

## Configuration
Thresholds for stationary detection (±0.02g), movement (0.06g), and debounce (200ms) are defined in `motion_monitor.c` and can be tuned for specific elevator models.
