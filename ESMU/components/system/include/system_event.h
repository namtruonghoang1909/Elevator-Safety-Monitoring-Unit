/**
 * @file system_event.h
 * @brief System-wide event definitions
 */

#pragma once
/**
 * @brief System event IDs
 */

typedef enum {
    SYSTEM_EVENT_BOOT,
    SYSTEM_EVENT_INITIALIZED,
    SYSTEM_EVENT_START_CONFIGURATION,
    SYSTEM_EVENT_DEVICE_CONFIGURATED,
    SYSTEM_EVENT_ELEVATOR_FAULT_DETECTED,
    SYSTEM_EVENT_ERROR
} system_event_id_t;


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
 * @brief System event structure type
 */
typedef struct {
    system_event_id_t id;
    system_error_id_t error;
} system_event_t;

void system_report_event(system_event_id_t event_id);
/**
 * @brief 
 * Post an error event to the system event queue
 */
void system_report_error(system_error_id_t error_id);