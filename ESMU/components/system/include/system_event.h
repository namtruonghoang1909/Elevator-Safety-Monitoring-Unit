#pragma once

#include "system_error.h"

/**
 * @brief System event IDs
 */
typedef enum system_event_id_t {
    SYSTEM_EVENT_BOOT,

    SYSTEM_EVENT_INITIALIZED,

    SYSTEM_EVENT_START_CONFIGURATION,
 
    SYSTEM_EVENT_DEVICE_CONFIGURATED,

    SYSTEM_EVENT_ELEVATOR_FAULT_DETECTED,

    SYSTEM_EVENT_ERROR
} system_event_id_t;

/**
 * @brief System event structure
 */
typedef struct system_event_t {
    system_event_id_t id;
    system_error_id_t error;  // Used if event is SYSTEM_EVENT_ERROR
} system_event_t;

/**
 * @brief 
 * Post an event to the system event queue
 * @param event_id The event ID to post
 */
void system_report_event(system_event_id_t event_id);