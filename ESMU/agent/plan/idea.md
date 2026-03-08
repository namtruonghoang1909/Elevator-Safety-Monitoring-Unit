# Motion Monitor Re-implementation Idea

## Objective
Re-implement the `motion_monitor` service to improve modularity, testability, and add elevator balance (tilt) detection.

## 1. Architectural Changes: Decoupling Logic from Service
Currently, `motion_monitor.c` handles both the FreeRTOS task and the processing logic. We will decouple these:
- **Service Layer (`motion_monitor.c`):** Handles initialization, task creation, mutex locking, and public API calls.
- **Task Layer (`motion_task.c`):** Implements the FreeRTOS task loop that polls the sensor and calls the processor.
- **Processor Layer (`motion_processor.c`):** A pure logic orchestrator that takes raw sensor data and updates metrics. This layer is independent of FreeRTOS.

## 2. New Feature: Balance (Tilt) Monitoring
Add a module to detect if the elevator is tilting.
- **`motion_balance.c`:** Calculates tilt angles based on filtered X and Y accelerometer data.
- **`balance_state_t`:**
    ```c
    typedef enum {
        BALANCE_LEVEL = 0,
        BALANCE_TILT_LEFT,
        BALANCE_TILT_RIGHT,
        BALANCE_TILT_FORWARD,
        BALANCE_TILT_BACKWARD
    } balance_state_t;
    ```
- **Logic:** Compare Accel X/Y values against a threshold (e.g., ±0.1g for roughly ±6 degrees).

## 3. Proposed File Structure
```text
components/services/motion_monitor/
├── include/
│   └── motion_monitor.h      (Public API & Unified Data Structs)
├── src/
│   ├── motion_monitor.c      (Init & API Implementation)
│   ├── motion_task.c         (FreeRTOS Task & Sync)
│   ├── motion_processor.c    (Logic Orchestrator)
│   ├── motion_filter.c       (Signal Smoothing)
│   ├── motion_fsm.c          (Elevator Motion State)
│   └── motion_balance.c      (New Tilt Detection)
└── CMakeLists.txt            (Updated for new files)
```

## 4. Key Benefits
- **Testability:** `motion_processor.c` can be tested in a host-based environment or via Unity tests without needing FreeRTOS.
- **Maintainability:** Each module (FSM, Filter, Balance) is isolated and easy to update.
- **Extensibility:** New metrics or logic (e.g., vibration FFT) can be added to the processor without changing the task or API structure.
