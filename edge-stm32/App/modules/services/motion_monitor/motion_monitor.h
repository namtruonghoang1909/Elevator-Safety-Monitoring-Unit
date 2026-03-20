/**
 * @file motion_monitor.h
 * @brief Motion Monitor service for ESMU Edge Node
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "mpu6050.h"

// ─────────────────────────────────────────────
// Types
// ─────────────────────────────────────────────

typedef enum {
    MOTION_STATE_STATIONARY = 0,
    MOTION_STATE_MOVING_UP,
    MOTION_STATE_MOVING_DOWN,
    MOTION_STATE_SHAKING,
    MOTION_STATE_FREE_FALL
} motion_state_t;

typedef struct {
    motion_state_t state;
    float avg_tilt;        /**< EMA smoothed tilt magnitude */
    float max_tilt;        /**< Peak tilt in current window */
    uint8_t health_score;  /**< 0-100 calculated score */
} motion_metrics_t;

typedef struct {
    float filter_alpha;    /**< EMA alpha (0.0 to 1.0) */
} motion_monitor_config_t;

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

/**
 * @brief Initialize motion monitor
 */
bool motion_monitor_init(const motion_monitor_config_t *cfg);

/**
 * @brief Start the internal FreeRTOS task
 */
void motion_monitor_start(void);

/**
 * @brief Get current metrics and state
 */
void motion_monitor_get_metrics(motion_metrics_t *metrics);

/**
 * @brief Perform Z-axis calibration (Capture gravity offset)
 */
void motion_monitor_calibrate(void);
