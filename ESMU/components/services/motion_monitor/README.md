# Motion Monitor Service

The `motion_monitor` service is the "Inner Ear" of the Elevator Safety Monitoring Unit (ESMU). It processes high-frequency raw data from the MPU6050 sensor into stable, high-level "elevator concepts" such as motion state, horizontal shake, and car balance (tilt).

## 🚀 Key Features

- **Noise Reduction**: Employs an Exponential Moving Average (EMA) filter to eliminate motor and rail jitter.
- **Gravity Compensation**: Calibration logic isolates the 1.0g gravity vector to provide pure vertical linear acceleration ($Linear\ Z$).
- **Vibration Analysis**: Calculates the Euclidean magnitude ($\sqrt{A_x^2 + A_y^2}$) of horizontal forces to monitor ride quality.
- **Balance Detection (New)**: Detects if the elevator car is leaning (Left, Right, Forward, Backward) based on filtered gravity shift on X/Y axes.
- **Debounced State Machine**: Tracks elevator trips through logical states: `STATIONARY`, `MOVING_UP/DOWN`, and `DECELERATING_UP/DOWN`.

## 🏗️ Modular Architecture

The service is decoupled into logical modules for better testability and maintenance:

| Module | Location | Responsibility |
| :--- | :--- | :--- |
| **Public API** | `motion_monitor.c` | External interface and context management. |
| **Task Layer** | `src/core/motion_task.c` | FreeRTOS task, sensor polling (100Hz), and Mutex locking. |
| **Processor** | `src/core/motion_processor.c` | **Pure Logic** orchestrator (Filter -> FSM -> Balance). |
| **Filter** | `src/filter/motion_filter.c` | Signal smoothing (EMA) and basic math. |
| **FSM** | `src/fsm/motion_fsm.c` | Elevator movement state transitions and debouncing. |
| **Balance** | `src/balance/motion_balance.c` | Tilt detection based on X/Y gravity vectors. |

## 📊 Data Types

### Motion Metrics (`motion_metrics_t`)
```c
typedef struct {
    float lin_accel_z;      /**< Vertical acceleration (g) - Gravity removed */
    float shake_mag;        /**< Horizontal vibration (g) - sqrt(X^2 + Y^2) */
    float gyro_roll_dps;    /**< Filtered angular velocity around X (°/s) */
    float gyro_pitch_dps;   /**< Filtered angular velocity around Y (°/s) */
    motion_state_t state;   /**< Current movement state (Moving/Stationary) */
    balance_state_t balance;/**< Current car balance (Level/Tilted) */
    uint32_t last_update;   /**< System timestamp (ms) */
} motion_metrics_t;
```

## 🛠️ Public API

- **`motion_monitor_init(cfg)`**: Initializes the mutex, processor, and starts the 100Hz polling task.
- **`motion_monitor_get_metrics(out)`**: Thread-safe retrieval of all processed data.
- **`motion_monitor_calibrate()`**: Blocking 2-second sequence to "zero" the gravity vector. **Must be stationary.**
- **`motion_monitor_inject_sample(raw)`**: Logic-only injection for unit testing (bypasses hardware).

## 🧪 Testing & Verification

### Logic Unit Tests (Unity)
The "Processor" layer is decoupled from FreeRTOS, allowing for precise logic verification via data injection.
- **Location**: `test/test_motion_monitor/`
- **Command**: `pio test -e esp32doit-devkit-v1 -f test_motion_monitor`

### Integration Logging
Use `motion_monitor_get_metrics()` in your main loop to log elevator behavior:
```c
motion_metrics_t m;
if (motion_monitor_get_metrics(&m) == ESP_OK) {
    ESP_LOGI("APP", "State: %d | Balance: %d | Shake: %.2f g", 
             m.state, m.balance, m.shake_mag);
}
```

## ⚙️ Configuration
Thresholds for stationary detection (±0.02g), movement (0.06g), and tilt (0.15g) are tunable within their respective modules in the `src/` directory.
