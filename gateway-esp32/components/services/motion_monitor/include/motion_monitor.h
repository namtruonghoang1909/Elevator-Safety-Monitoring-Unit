/**
 * @file motion_monitor.h
 * @brief Motion Monitor service (Remote Proxy version for Distributed ESMU)
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Logical elevator motion states
 */
typedef enum {
    MOTION_STATE_STATIONARY,        /**< Elevator is not moving */
    MOTION_STATE_MOVING_UP,          /**< Elevator is ascending */
    MOTION_STATE_DECELERATING_UP,    /**< Elevator is braking while going up */
    MOTION_STATE_MOVING_DOWN,        /**< Elevator is descending */
    MOTION_STATE_DECELERATING_DOWN,  /**< Elevator is braking while going down */
    MOTION_STATE_ACCELERATING        /**< Generic high-G transient */
} motion_state_t;

/**
 * @brief Elevator balance (tilt) states
 */
typedef enum {
    BALANCE_STATE_LEVEL = 0,         /**< Elevator is level within threshold */
    BALANCE_STATE_TILT_LEFT,         /**< Tilted towards negative X */
    BALANCE_STATE_TILT_RIGHT,        /**< Tilted towards positive X */
    BALANCE_STATE_TILT_FORWARD,      /**< Tilted towards positive Y */
    BALANCE_STATE_TILT_BACKWARD      /**< Tilted towards negative Y */
} balance_state_t;

/**
 * @brief Processed motion metrics (Received from Remote Node)
 */
typedef struct {
    float lin_accel_z;      /**< Linear vertical acceleration (g) */
    float shake_mag;        /**< Horizontal vibration magnitude (g) */
    float gyro_roll_dps;    /**< Angular velocity around X (°/s) */
    float gyro_pitch_dps;   /**< Angular velocity around Y (°/s) */
    motion_state_t state;   /**< Logical elevator movement state */
    balance_state_t balance;/**< Logical elevator balance state */
    uint32_t last_update;   /**< Timestamp of last calculation (ms) */
} motion_metrics_t;

/**
 * @brief Motion monitor configuration
 */
typedef struct {
    uint32_t stats_update_ms; /**< Interval for status checks (ms) */
} motion_monitor_config_t;

/**
 * @brief Initialize the motion monitor service (Remote Sink mode)
 * 
 * @param cfg Configuration parameters
 * @return ESP_OK on success
 */
esp_err_t motion_monitor_init(const motion_monitor_config_t *cfg);

/**
 * @brief Update the internal state with metrics received from CAN/Remote node
 * 
 * @param metrics Pointer to the latest metrics received from the edge node
 * @return ESP_OK on success
 */
esp_err_t motion_monitor_update(const motion_metrics_t *metrics);

/**
 * @brief Get a thread-safe copy of the latest motion metrics
 * 
 * @param out Pointer to store the metrics
 * @return ESP_OK on success
 */
esp_err_t motion_monitor_get_metrics(motion_metrics_t *out);

/**
 * @brief Get the current logical elevator motion state
 * 
 * @return motion_state_t 
 */
motion_state_t motion_monitor_get_motion(void);

/**
 * @brief Get the current logical elevator balance state
 * 
 * @return balance_state_t 
 */
balance_state_t motion_monitor_get_equilibrium(void);

/**
 * @brief Check if the remote motion data is fresh
 * 
 * @param timeout_ms Maximum allowed age of the data
 * @return true if fresh, false if stale/offline
 */
bool motion_monitor_is_alive(uint32_t timeout_ms);
