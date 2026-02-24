#pragma once

/**
 * @brief System states
 */
typedef enum system_state_id_t {
    SYSTEM_STATE_IDLE,

    SYSTEM_STATE_INITIALIZING,

    SYSTEM_STATE_CONFIGURING, // intend to be configurated through a web server

    SYSTEM_STATE_MONITORING,
    
    SYSTEM_STATE_ERROR
} system_state_id_t;

/**
 * @brief 
 * - Initialize the system module
 *      - NVS
 *      - event loop
 *      - xqueue storing system events
 */
void system_start(void);
