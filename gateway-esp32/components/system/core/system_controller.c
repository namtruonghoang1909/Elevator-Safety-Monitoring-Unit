#include "system.h"
#include "system_config.h"
#include "system_event.h"
#include "system_registry.h"
#include "system_hw.h"
#include "system_boot.h"
#include "heartbeat.h"
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "SYS_CTRL";

static system_state_id_t current_system_state = SYSTEM_STATE_IDLE;
static QueueHandle_t system_event_queue = NULL;

/***********************************************************************
 * STATIC FUNCTION DECLARATIONS
 */
static void system_task(void *pvParameters);
static void system_event_handler(system_event_t new_event);
static bool is_system_at_state(system_state_id_t state);
static void state_transition(system_state_id_t new_state);

/***********************************************************************
 * STATIC FUNCTION DEFINITIONS
 */

static bool is_system_at_state(system_state_id_t state) {
    return current_system_state == state;
}

static void state_transition(system_state_id_t new_state) {
    if (current_system_state == new_state) return;
    
    ESP_LOGI(TAG, "FSM Transition: %d -> %d", current_system_state, new_state);
    current_system_state = new_state;
    system_registry_set_state(new_state);
}

static void system_task(void *pvParameters){
    system_event_t new_evt;
    ESP_LOGI(TAG, "System Supervisor Task Started");
    
    while(1) {
        if(xQueueReceive(system_event_queue, &new_evt, portMAX_DELAY)){
            system_event_handler(new_evt);
        }
    }
}

static void system_event_handler(system_event_t new_event) {
    // Global Error Handling
    if (new_event.id == SYSTEM_EVENT_ERROR) {
        state_transition(SYSTEM_STATE_ERROR);
        return;
    }

    switch(new_event.id) {
        case SYSTEM_EVENT_BOOT:
            if(is_system_at_state(SYSTEM_STATE_IDLE)){
                state_transition(SYSTEM_STATE_INITIALIZING);
                // Trigger Service Initialization Sequence
                system_boot_begin();
            } 
            break;

        case SYSTEM_EVENT_INITIALIZED:
            if(is_system_at_state(SYSTEM_STATE_INITIALIZING)){
                state_transition(SYSTEM_STATE_MONITORING);
            }
            break;
            
        case SYSTEM_EVENT_START_CONFIGURATION:
            state_transition(SYSTEM_STATE_CONFIGURING);
            break;

        case SYSTEM_EVENT_DEVICE_CONFIGURATED:
            if(is_system_at_state(SYSTEM_STATE_CONFIGURING)){
                state_transition(SYSTEM_STATE_MONITORING); 
                system_registry_set_subtext("WiFi Configured");
            }
            break;

        default:
            break;
    }
}

/***********************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 */
esp_err_t system_core_init(void) {
    if (system_event_queue != NULL) return ESP_OK;

    // 1. Initialize System Registry
    esp_err_t esp_ret = system_registry_init();
    if(esp_ret != ESP_OK){
        ESP_LOGE(TAG, "System registry initialization failed");
        return esp_ret;
    }

    // 2. Start Visual Heartbeat (Registry must be initialized first)
    heartbeat_init();

    // 3. Initialize Hardware Abstractions (Buses, Pins)
    esp_ret = system_hw_init();
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Hardware initialization failed");
        return esp_ret;
    }

    // 3. Create Event Queue
    system_event_queue = xQueueCreate(20, sizeof(system_event_t));
    if (system_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return ESP_ERR_NO_MEM;
    }
    
    ESP_LOGI(TAG, "System Controller Init Complete (Queue Created)");

    // 4. Start System Controller Task
    BaseType_t ret = xTaskCreate(system_task, "system_task", 4096, NULL, 10, NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "System controller task failed");
        return ESP_FAIL;
    }



    return ESP_OK;
}

void system_report_error(system_error_id_t err_id) {
    if (system_event_queue == NULL) return;
    system_event_t evt = { .id = SYSTEM_EVENT_ERROR, .error = err_id };
    xQueueSend(system_event_queue, &evt, 0);
}    

void system_report_event(system_event_id_t event_id) {
    if (system_event_queue == NULL) return;
    system_event_t evt = { .id = event_id, .error = SYSTEM_ERROR_NONE };
    xQueueSend(system_event_queue, &evt, 0);
}
