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
    FAULT_OVERTILT              /**< Elevator tilt exceeds safety limits */
} fault_code_t;

/**
 * @brief Logical elevator balance (tilt) states
 */
typedef enum {
    BALANCE_STATE_LEVEL = 0,
    BALANCE_STATE_TILT_LEFT,
    BALANCE_STATE_TILT_RIGHT,
    BALANCE_STATE_TILT_FORWARD,
    BALANCE_STATE_TILT_BACKWARD
} balance_state_t;

/**
 * @brief Edge node health status (Node Hardware/Internal Faults)
 */
typedef enum {
    EDGE_HEALTH_OK = 0,
    EDGE_HEALTH_SENSOR_LOST,    /**< MPU6050 communication failed */
    EDGE_HEALTH_CAN_FAIL,       /**< CAN controller error */
    EDGE_HEALTH_LOW_VOLTAGE     /**< Power supply issues */
} edge_health_t;
