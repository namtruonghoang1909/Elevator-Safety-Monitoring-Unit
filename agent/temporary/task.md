# Task: SIM A7680C Driver Verification & Service Flow Design

## Goal:
Thoroughly verify the refactored `sim_a7680c` driver and design the logic for the upcoming `cellular_service`.

## Plan:

### 1. Driver Verification (Current Focus)
- [ ] Update `main.c` to include a sequential test runner:
    - [ ] Hardware Reset & Sync.
    - [ ] Comprehensive Info Retrieval (Model, IMEI, IMSI).
    - [ ] Registration Loop (Wait for Network).
    - [ ] SMS Test (Controlled single-shot).
    - [ ] Voice Call Test (Controlled single-shot with hang-up).
- [ ] Monitor logs for:
    - [ ] URC (Unsolicited Result Code) interference.
    - [ ] Timeout reliability.
    - [ ] Mutex contention between tasks (if any).

### 2. Service Flow Design
- [ ] Analyze driver responses to design a robust FSM (Finite State Machine).
- [ ] Define recovery strategies for:
    - [ ] SIM Not Inserted.
    - [ ] Registration Denied.
    - [ ] Signal Loss (Low CSQ).
    - [ ] Module Freeze (UART Timeout).
- [ ] Document the design in `agent/plan/idea.md`.

### 3. Cleanup & Checkpoint
- [ ] Revert `main.c` to a stable monitoring state after testing.
- [ ] Update `agent/context/checkpoint.md` with findings.
