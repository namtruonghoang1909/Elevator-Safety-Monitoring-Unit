#include <math.h>
#include "esp_log.h"
#include "motion_fsm.h"

static const char *TAG = "MM_FSM";

#define THRESHOLD_STATIONARY_G  0.02f
#define THRESHOLD_MOVING_G      0.06f
#define THRESHOLD_ACCEL_G       0.15f
#define DEBOUNCE_TARGET_MS      200

motion_state_t mm_fsm_update(motion_state_t current_state, 
                             float lin_accel_z, 
                             uint32_t *timer_ms, 
                             motion_state_t *pending_state) 
{
    motion_state_t target = current_state;
    float abs_z = fabsf(lin_accel_z);

    // 1. Logic to pick target based on physics
    switch (current_state) {
        case MOTION_STATE_STATIONARY:
            if (lin_accel_z > THRESHOLD_MOVING_G) target = MOTION_STATE_MOVING_UP;
            else if (lin_accel_z < -THRESHOLD_MOVING_G) target = MOTION_STATE_MOVING_DOWN;
            else if (abs_z > THRESHOLD_ACCEL_G) target = MOTION_STATE_ACCELERATING;
            break;

        case MOTION_STATE_MOVING_UP:
            if (lin_accel_z < -THRESHOLD_MOVING_G) target = MOTION_STATE_DECELERATING_UP;
            else if (abs_z < THRESHOLD_STATIONARY_G) target = MOTION_STATE_STATIONARY;
            break;

        case MOTION_STATE_DECELERATING_UP:
            if (abs_z < THRESHOLD_STATIONARY_G) target = MOTION_STATE_STATIONARY;
            break;

        case MOTION_STATE_MOVING_DOWN:
            if (lin_accel_z > THRESHOLD_MOVING_G) target = MOTION_STATE_DECELERATING_DOWN;
            else if (abs_z < THRESHOLD_STATIONARY_G) target = MOTION_STATE_STATIONARY;
            break;

        case MOTION_STATE_DECELERATING_DOWN:
            if (abs_z < THRESHOLD_STATIONARY_G) target = MOTION_STATE_STATIONARY;
            break;

        case MOTION_STATE_ACCELERATING:
            if (abs_z < THRESHOLD_STATIONARY_G) target = MOTION_STATE_STATIONARY;
            break;

        default:
            target = MOTION_STATE_STATIONARY;
            break;
    }

    // 2. Debounce Implementation
    if (target == current_state) {
        *timer_ms = 0;
        *pending_state = current_state;
        return current_state;
    }

    if (target != *pending_state) {
        *pending_state = target;
        *timer_ms = 0;
    } else {
        *timer_ms += 10; // Assume 100Hz caller
        if (*timer_ms >= DEBOUNCE_TARGET_MS) {
            ESP_LOGI(TAG, "State Transition: %d -> %d", current_state, target);
            *timer_ms = 0;
            return target;
        }
    }

    return current_state;
}
