#include "system.h"
#include "system_config.h"
#include "system_event.h"
#include "system_registry.h"
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "display_service.h"
#include "connectivity_manager.h"
#include "motion_proxy.h"
#include "telemetry_service.h"
#include "can_platform.h"
#include "i2c_platform.h"
#include "nvs_flash.h"

static const char *TAG = "SYS_CTRL";

static system_state_id_t current_system_state = SYSTEM_STATE_IDLE;
static QueueHandle_t system_event_queue = NULL;

/***********************************************************************
 * STATIC FUNCTION DECLARATIONS
 */
static esp_err_t system_init();
static void system_task(void *pvParameters);
static void system_event_handler(system_event_t new_event);
static bool is_system_at_state(system_state_id_t state);
static void state_transition(system_state_id_t new_state);

/***********************************************************************
 * STATIC FUNCTION DEFINITIONS
 */
static esp_err_t system_init()
{
    // 1. Platform Init (Needed for SSD1306)
    uint8_t bus_id;
    esp_err_t ret = i2c_bus_init(&bus_id, 21, 22);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. Manual Initialization Sequence (Since we are in INITIALIZING now)
    ret = display_service_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display service init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    system_registry_set_subtext("Booting ESMU Gateway...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 3. Connectivity Services
    system_registry_set_subtext("Connectivity Init...");
    connectivity_config_t conn_cfg = {
        .wifi_config = { 
            .ssid = NULL,       // Let NVS/Provisioning handle it
            .password = NULL,
            .auto_reconnect = true,
        },
        .mqtt_config = { 
            .broker_uri = BROKER_URI, 
            .client_id = CLIENT_ID,
            .username = CLIENT_USERNAME,
            .password = CLIENT_PASSWORD,
            .port = 1883,
            .disable_auto_reconnect = false,
        },
    };
    ret = connectivity_manager_init(&conn_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Connectivity manager init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ret = connectivity_manager_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Connectivity manager start failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. CAN Platform Init
    system_registry_set_subtext("CAN Bus Init...");
    can_config_t can_cfg = {
        .tx_pin = CAN_TX_PIN,
        .rx_pin = CAN_RX_PIN,
        .baud_rate_kbps = CAN_BAUD_RATE_KBPS,
        .mode = CAN_MODE_NORMAL // Default to normal for production
    };
    ret = can_init(&can_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CAN init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    can_start();

    // 5. Motion Proxy (Remote Sink Mode)
    system_registry_set_subtext("Waiting for Edge Node...");
    ret = motion_proxy_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Motion proxy init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 6. Telemetry Service
    ret = telemetry_service_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Telemetry service init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1500));

    // 6. Monitoring Phase
    system_registry_set_subtext("Gateway Active");
    system_report_event(SYSTEM_EVENT_INITIALIZED);

    return ESP_OK;
}

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
                system_init();
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

    system_event_queue = xQueueCreate(20, sizeof(system_event_t));
    if (system_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return ESP_ERR_NO_MEM;
    }
    
    ESP_LOGI(TAG, "System Controller Init Complete (Queue Created)");

    BaseType_t ret = xTaskCreate(system_task, "system_task", 4096, NULL, 10, NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "System controller task failed");
        return ESP_FAIL;
    }

    esp_err_t esp_ret = system_registry_init();
    if(esp_ret != ESP_OK){
        ESP_LOGE(TAG, "System registry initialization failed");
        return esp_ret;
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
