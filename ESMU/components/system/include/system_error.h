#pragma once

/**
 * @brief System error IDs
 */
typedef enum system_error_id_t {
    SYSTEM_ERROR_NONE = 0,

    /* Boot Critical */
    SYSTEM_ERROR_MEMORY_ALLOCATION_FAILED,
    SYSTEM_ERROR_TASK_CREATION_FAILED,
    SYSTEM_ERROR_QUEUE_CREATION_FAILED,
    SYSTEM_ERROR_NVS_INIT_FAILED,

    /* Core Modules */
    SYSTEM_ERROR_SENSOR_INIT_FAILED,
    SYSTEM_ERROR_ALERT_SUBSYSTEM_FAILED,

    /* Capacity Issues */
    SYSTEM_ERROR_FAULT_STORAGE_FULL,

} system_error_id_t;

/**
 * @brief 
 * Post an error event to the system event queue
 */
void system_report_error(system_error_id_t error_id);
