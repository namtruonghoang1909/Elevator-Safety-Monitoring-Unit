#include "system.h"
#include "system_event.h"
#include "system_error.h"
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define FSM_WARN_UNEXPECTED(evt) \
    ESP_LOGW(TAG, "Event %d ignored in state %d", evt, current_system_state)

/***********************************************************************
 * Internal variables
 *********************************************************************** 
 */
static const char *TAG = "SYSTEM_CONTROLLER";

static system_state_id_t current_system_state = SYSTEM_STATE_IDLE;

static QueueHandle_t system_event_queue;

/***********************************************************************
 * Internal function declarations
 ***********************************************************************
 */
/**
 * @brief
 * system task to process system events
 * @param pvParameters Parameters for the task (not used)
 * @return void
 */
static void system_task(void *pvParameters);

/**
 * @brief 
 * system intialization
 *  - create event handler task
 *  - initalize hardware
 */
static void system_init();
/**
 * @brief
 * Handle a new system event
 * @param new_event The new system event to handle
 * @return void
 */
static void system_event_handler(system_event_t new_event);

/**
 * @brief
 * Handle a system error
 * @param error_id The error ID to handle
 * @return void
 */
static void system_error_handler(system_error_id_t error_id);


/***********************************************************************
 * Internal helper functions declarations
 ***********************************************************************
 */
static bool is_system_at_state(system_state_id_t state) {
    return current_system_state == state;
}

static void state_transition(system_state_id_t new_state) {
    ESP_LOGI(TAG, "System state transition: %d -> %d",
             current_system_state, new_state);
    current_system_state = new_state;
}

/***********************************************************************
 * Global function definitions
 ***********************************************************************
 */
void system_report_error(system_error_id_t err_id) {
    // Implementation for reporting the error
    // This could involve logging the error, sending it to a monitoring system, etc.
    system_event_t evt = {
        .id = SYSTEM_EVENT_ERROR,
        .error = err_id
    };
    xQueueSend(system_event_queue, &evt, portMAX_DELAY);
}    

void system_report_event(system_event_id_t event_id) {
    // Implementation for reporting the event
    // This could involve logging the event, notifying other system components, etc.
    system_event_t evt = {
        .id = event_id,
        .error = SYSTEM_ERROR_NONE,
    };
    xQueueSend(system_event_queue, &evt, portMAX_DELAY);
}


/***********************************************************************
 * Internal function definitions
 * *********************************************************************
 */
static void system_task(void *pvParameters){
    system_event_t new_evt;
    while(1)
    {
        if(xQueueReceive(system_event_queue, &new_evt, portMAX_DELAY)){
            system_event_handler(new_evt);
        }
    }
}

static void system_init()
{
    xTaskCreate(system_task, "system_task", 4096, NULL, 5, NULL);
    // hardware init() ...
}
static void system_event_handler(system_event_t new_event)
{
    // Ignore all events except ERROR when in ERROR state
    // We dont want to transition out of ERROR state without explicit recovery
    if (current_system_state == SYSTEM_STATE_ERROR &&
        new_event.id != SYSTEM_EVENT_ERROR) {

        ESP_LOGW(TAG,
            "FSM frozen. Ignoring event %d in ERROR state",
            new_event.id);
        return;
    }

    switch(new_event.id){

        /* ---------- BOOT & INIT ---------- */
        case SYSTEM_EVENT_BOOT:
            ESP_LOGI(TAG, "System boot event received");
            if(is_system_at_state(SYSTEM_STATE_IDLE)){
                system_init();
                state_transition(SYSTEM_STATE_INITIALIZING);
            } 
            else {
                FSM_WARN_UNEXPECTED(new_event.id);
            }
            break;

        case SYSTEM_EVENT_INITIALIZED:
            ESP_LOGI(TAG, "System initialized event received");
            if(is_system_at_state(SYSTEM_STATE_INITIALIZING)){
                //connectivity_start(); 
                // system only initializes and performs telemetry \n
                // when connectivity is configured(this will be checked in the connectivity_start() function)
                // after the connectivity is started, telemetry can start
                state_transition(SYSTEM_STATE_MONITORING);
            }
            else {
                FSM_WARN_UNEXPECTED(new_event.id);
            }
            break;            

        /* ---------- CONFIGURATION ---------- */
        case SYSTEM_EVENT_START_CONFIGURATION: // user starts configuration through web server
            ESP_LOGI(TAG, "Start configuration event received");
            if(is_system_at_state(SYSTEM_STATE_MONITORING)){
                // start web server task here
                //connectivity_stop();
                // attempt to stop telemetry then stop wifi connection if task available
                state_transition(SYSTEM_STATE_CONFIGURING);
            }
            else {
                FSM_WARN_UNEXPECTED(new_event.id);
            } 
            break;

        case SYSTEM_EVENT_DEVICE_CONFIGURATED: // user finishes configuration through web server
            ESP_LOGI(TAG, "Device configurated event received");
            if(is_system_at_state(SYSTEM_STATE_CONFIGURING)){
                //connectivity_start(); 
                // system only initializes and performs telemetry \n
                // when connectivity is configured(this will be checked in the connectivity_start() function)
                // after the connectivity is started, telemetry can start

                state_transition(SYSTEM_STATE_MONITORING);
            }
            else {
                FSM_WARN_UNEXPECTED(new_event.id);
            } 
            break;

        /* ---------- FAULT DETECTION ---------- */
        case SYSTEM_EVENT_ELEVATOR_FAULT_DETECTED:
            ESP_LOGI(TAG, "Elevator fault detected event received");
            if(is_system_at_state(SYSTEM_STATE_MONITORING)){
                //device_send_alert(); 
                // sms and mqtt for fatal faults, sms to phone and mqtt on emergency topic
                // emergency topic only being sent when connected to broker
                // sms to phone performs only when sms infos are configured
                state_transition(SYSTEM_STATE_MONITORING);
            }
            else {
                ESP_LOGW(TAG, "Another fault event arrived when system is not at MONITORING!");
                FSM_WARN_UNEXPECTED(new_event.id);
            } 
            break;

        /* ---------- ERROR ---------- */
        case SYSTEM_EVENT_ERROR:
            ESP_LOGI(TAG, "System error event received: %d", new_event.error);
            state_transition(SYSTEM_STATE_ERROR);
            system_error_handler(new_event.error);
            break;
        
        default:
            ESP_LOGW(TAG, "Unknown system event received: %d", new_event.id);
            break;
    }
}

static void system_error_handler(system_error_id_t error_id) {
    switch (error_id) {
        case SYSTEM_ERROR_NONE:
        break;
        case SYSTEM_ERROR_MEMORY_ALLOCATION_FAILED:
        break;
        case SYSTEM_ERROR_TASK_CREATION_FAILED:
        break;
        case SYSTEM_ERROR_QUEUE_CREATION_FAILED:
        break;
        case SYSTEM_ERROR_NVS_INIT_FAILED:
        break;
        case SYSTEM_ERROR_SENSOR_INIT_FAILED:
        break;
        case SYSTEM_ERROR_ALERT_SUBSYSTEM_FAILED:
        break;
        case SYSTEM_ERROR_FAULT_STORAGE_FULL:
        break;
    }
}

