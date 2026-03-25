/**
 * @file protocol_types.h
 * @brief Shared enums and constants for ESMU distributed protocol
 */

#pragma once

#include <stdint.h>

/**
 * @brief Edge node operational states
 */
typedef enum {
    EDGE_STATE_INIT = 0,      /**< Node is initializing or calibrating sensors */
    EDGE_STATE_RUNNING,       /**< Node is actively monitoring and healthy */
    EDGE_STATE_ERROR          /**< Node has encountered a critical hardware error */
} edge_state_t;

/**
 * @brief Critical fault types detected by the Edge node (Physical Elevator Faults)
 */
typedef enum {
    FAULT_NONE = 0,
    FAULT_SHAKE,                /**< Excessive vibration detected */
    FAULT_FREEFALL,             /**< Zero-G condition detected */
    FAULT_EMERGENCY_STOP,       /**< Sharp deceleration/impact detected */
} fault_code_t;

/**
 * @brief Multi-level health status for real-time monitoring
 */
typedef enum {
    HEALTH_STABLE = 0,      /**< System operating within normal parameters */
    HEALTH_WARNING,         /**< Thresholds exceeded (e.g., high vibration/tilt) */
    HEALTH_EMERGENCY        /**< Critical fault detected (Free fall/Impact) */
} health_status_t;

/**
 * @brief Edge node health status (Node Hardware/Internal Faults)
 */
typedef enum {
    EDGE_HEALTH_OK = 0,
    EDGE_HEALTH_SENSOR_LOST,    /**< MPU6050 communication failed */
    EDGE_HEALTH_CAN_FAIL,       /**< CAN controller error */
    EDGE_HEALTH_LOW_VOLTAGE     /**< Power supply issues */
} edge_health_t;

/**
 * @brief Logical elevator motion states
 */
typedef enum {
    MOTION_STATE_STATIONARY = 0,
    MOTION_STATE_MOVING_UP,
    MOTION_STATE_MOVING_DOWN,
    MOTION_STATE_SHAKING,
    MOTION_STATE_FREE_FALL
} motion_state_t;
