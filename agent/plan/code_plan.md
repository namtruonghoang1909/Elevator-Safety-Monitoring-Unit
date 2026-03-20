# ESMU Edge Node (STM32) - System Registry & Whiteboard Architecture

## Objective
Implement a thread-safe "Whiteboard" (System Registry) architecture for the Edge Node. This design allows the `MotionMonitor` to publish high-frequency health data to a shared memory area, which the `DisplayService` and `CANService` can then consume at their own rates.

---

## 1. The Whiteboard (System Registry)

A central component (`system_registry.c`) will hold the current state of the elevator.

### A. Shared Data Structure
```c
typedef struct {
    motion_metrics_t metrics;    // Tilt, Acceleration, Health Score
    mpu6050_scaled_data_t raw;   // Real-time G-forces
    node_state_t state;          // INIT, MONITORING, EMERGENCY, ERROR
    uint32_t last_update_tick;   // For watchdog/freshness checks
} system_registry_t;
```

### B. Concurrency Control
- **Mutex**: A single Mutex (`registry_mutex`) will protect the entire structure.
- **APIs**:
    - `registry_write(system_registry_t *data)`: Thread-safe update.
    - `registry_read(system_registry_t *out_data)`: Thread-safe read.

---

## 2. Task Architecture (Registry-Based)

### A. Motion Task (The Writer)
- **Rate**: 100Hz (10ms).
- **Logic**:
    1. Read MPU6050.
    2. Process filters and detect states.
    3. **Registry Write**: Update the Whiteboard with new metrics and raw data.

### B. Display Task (The Reader)
- **Rate**: 10Hz (100ms).
- **Logic**:
    1. **Registry Read**: Fetch the latest data from the Whiteboard.
    2. Update SSD1306 OLED with the values retrieved.

### C. System Task (The Orchestrator)
- **Rate**: 10Hz (100ms).
- **Logic**:
    1. **Registry Read**: Check for emergencies or sensor timeouts.
    2. Handle state transitions (e.g., INIT -> MONITORING).
    3. (Future) Feed data from Registry into CAN packets.

---

## 3. Initialization & Lifecycle

### A. Sequence (in `main.c`)
1. **BSP Init**: HW initialization.
2. **System Core Init**:
    - `system_registry_init()`: Create the Mutex and initialize the Whiteboard memory.
    - `system_core_init()`: Create any additional IPC if needed.
3. **System Start**: Spawn `MotionTask`, `DisplayTask`, and `SystemTask`.
4. **Scheduler Start**: `osKernelStart()`.

---

## 4. Implementation Roadmap

### Phase 1: Registry Foundation
- [ ] Create `system_registry.h` and `system_registry.c`.
- [ ] Implement `registry_init`, `registry_read`, and `registry_write`.
- [ ] Implement `system_core_init()` and `system_start()` in a new `system.c`.

### Phase 2: Refactor Motion & Display
- [ ] **Motion Monitor**:
    - Strip all OLED calls.
    - Implement `registry_write` at the end of each 100Hz loop.
- [ ] **Display Service**:
    - Create `display_service.c`.
    - Implement `DisplayTask` to `registry_read` and draw the UI.

### Phase 3: System Logic
- [ ] Implement the 4-state FSM in `system.c` using the Registry as the source of truth.
- [ ] Add watchdog logic: If `last_update_tick` is too old, set State to **ERROR**.

---

## 5. Success Criteria
- [ ] Multiple tasks can access elevator health data without race conditions.
- [ ] Adding a new "consumer" (like CAN) is as simple as reading from the Whiteboard.
- [ ] `main.c` is minimal and clean.
- [ ] Hardware initialization occurs during the `INIT` state of the FSM.
