/**
 * @file fsm.h
 * @brief Elevator state machine logic and transition rules
 */

#pragma once

#include "motion_monitor.h"

/**
 * @brief Evaluates physics against thresholds to update logical state
 * 
 * Implements a debounced FSM that tracks trips (Stationary -> Moving -> Decelerating).
 * 
 * @param current_state The current logical state of the elevator
 * @param lin_accel_z Current vertical linear acceleration (gravity removed)
 * @param timer_ms [in/out] Pointer to persistent debounce timer (increments by 10ms)
 * @param pending_state [in/out] Pointer to persistent state candidate during debounce
 * @return motion_state_t The new logical state (either updated or unchanged)
 */
motion_state_t mm_fsm_update(motion_state_t current_state, 
                             float lin_accel_z, 
                             uint32_t *timer_ms, 
                             motion_state_t *pending_state);
