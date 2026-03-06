/**
 * @file motion_monitor.h
 * @brief Motion Monitor service for elevator movement and vibration analysis
 */

#pragma once

#include "esp_err.h"
#include "mpu6050.h"

/**
 * @brief Logical elevator motion states
 */
typedef enum {
    MOTION_STATE_STATIONARY,        /**< Elevator is not moving */
    MOTION_STATE_MOVING_UP,          /**< Elevator is ascending (Accel or Constant) */
    MOTION_STATE_DECELERATING_UP,    /**< Elevator is braking while going up */
    MOTION_STATE_MOVING_DOWN,        /**< Elevator is descending (Accel or Constant) */
    MOTION_STATE_DECELERATING_DOWN,  /**< Elevator is braking while going down */
    MOTION_STATE_ACCELERATING        /**< Generic high-G transient */
} motion_state_t;

/**
 * @brief Processed motion metrics (filtered and compensated)
 */
typedef struct {
    float lin_accel_z;    /**< Linear vertical acceleration (g) - Gravity removed */
    float shake_mag;      /**< Horizontal vibration magnitude (g) - sqrt(X^2 + Y^2) */
    float gyro_roll_dps;  /**< Filtered angular velocity around X (°/s) */
    float gyro_pitch_dps; /**< Filtered angular velocity around Y (°/s) */
    motion_state_t state; /**< Current logical elevator state */
    uint32_t last_update; /**< Timestamp of last calculation (ms) */
} motion_metrics_t;

/**
 * @brief Motion monitor configuration
 */
typedef struct {
    uint8_t mpu_dev_id;     /**< Initialized MPU6050 device ID */
    float filter_alpha;      /**< EMA filter alpha (0.0 - 1.0, default ~0.2) */
    uint32_t task_priority;  /**< FreeRTOS task priority */
    uint32_t task_stack;     /**< FreeRTOS task stack size (bytes) */
} motion_monitor_config_t;

/**
 * @brief Initialize the motion monitor service and start the background task
 * 
 * @param cfg Configuration parameters
 * @return ESP_OK on success
 */
esp_err_t motion_monitor_init(const motion_monitor_config_t *cfg);

/**
 * @brief Get a thread-safe copy of the latest motion metrics
 * 
 * @param out Pointer to store the metrics
 * @return ESP_OK on success
 */
esp_err_t motion_monitor_get_metrics(motion_metrics_t *out);

/**
 * @brief Get the current logical elevator state
 * 
 * @return motion_state_t 
 */
motion_state_t motion_monitor_get_state(void);

/**
 * @brief Perform a 2-second calibration sequence to "zero" the gravity vector
 * 
 * @note The unit must be STATIONARY during this call. This is a blocking call.
 * @return ESP_OK on success
 */
esp_err_t motion_monitor_calibrate(void);

/**
 * @brief Dynamically adjust the EMA filter responsiveness
 * 
 * @param alpha New alpha value (0.2 = smooth, 0.8 = fast)
 * @return ESP_OK on success
 */
esp_err_t motion_monitor_set_filter_alpha(float alpha);

/**
 * @brief Inject a raw sample for logic testing (bypasses hardware task)
 * 
 * @note Used only for Unity tests.
 * @param raw_sample Pointer to simulated raw scaled data
 * @return ESP_OK on success
 */
esp_err_t motion_monitor_inject_sample(const mpu6050_scaled_data_t *raw_sample);
