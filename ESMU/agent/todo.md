# ESMU Task Todo Management

This file tracks the specific, approved steps for current task execution.
---

## I. ACTIVE TASK EXECUTION
*Detailed sub-tasks for the current directive.*

### Task 08: Motion Monitor Unity Unit Test (Logic Verification)
Implement a comprehensive unit test for `motion_monitor` using data injection.

- [x] **1. Modify Motion Monitor for Testability**
    - [x] Refactored `_process_sample` for internal use.
    - [x] Added `motion_monitor_inject_sample` API.
    - [x] Added `TEST` mode support (disable hardware task if mpu_id=0xFF).
- [x] **2. Create Unity Test Suite**
    - [x] Created `test/test_motion_monitor/test_motion_monitor.c`.
    - [x] Implemented logic-only tests for Up/Down/Accel/Debounce.
- [x] **3. Verification**
    - [ ] Run the tests via `pio test`.

---
**Note**: To approve a task, reply in the chat with "Approved" or provide feedback on specific sub-tasks.
