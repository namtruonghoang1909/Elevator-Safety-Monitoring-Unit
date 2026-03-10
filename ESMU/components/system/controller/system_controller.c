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
#include "motion_monitor.h"
#include "i2c_platform.h"
#include "mpu6050.h"
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
    // 1. NVS init
    esp_err_t ret = nvs_flash_init();   
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ret = nvs_flash_erase();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "NVS erase failed: %s", esp_err_to_name(ret));
            return ret;
        }
        ret = nvs_flash_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "NVS re-init failed: %s", esp_err_to_name(ret));
            return ret;
        }
    } else if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. Platform Init
    uint8_t bus_id;
    ret = i2c_bus_init(&bus_id, 21, 22);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. Manual Initialization Sequence (Since we are in INITIALIZING now)
    ret = display_service_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display service init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    system_registry_set_subtext("Booting ESMU...");
    vTaskDelay(pdMS_TO_TICKS(1500));

    // 5. Sensor Init
    system_registry_set_subtext("Starting Sensors...");

    // 7. Production Services
    system_registry_set_subtext("Starting WiFi...");
    connectivity_config_t conn_cfg = {
        .wifi_config = { 
            .ssid = WIFI_SSID, 
            .password = WIFI_PASS, 
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

    // 8. Motion monitor init & Calibration Phase
    mpu6050_config_t mpu_cfg = {
        .address = 0x68,
        .name = "MPU6050",
        .scl_speed_hz = 400000,
        .bus_id = 0,
        .accel_full_scale = MPU6050_ACCEL_FS_8G,
        .gyro_full_scale = MPU6050_GYRO_FS_500_DPS,
        .use_gyro_pll = true,
        .enable_digital_filter = true,
    };
    uint8_t mpu_id;
    ret = mpu6050_init(&mpu_cfg, &mpu_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    system_registry_set_subtext("Lay device flat!");
    motion_monitor_config_t mm_cfg = {
        .mpu_dev_id = mpu_id,
        .filter_alpha = 0.2f,
        .task_priority = 5,
        .task_stack = 4096
    };
    ret = motion_monitor_init(&mm_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Motion monitor init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1500));
    system_registry_set_subtext("Calibrating...");
    ret = motion_monitor_calibrate();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Calibration failed: %s", esp_err_to_name(ret));
        // We continue anyway, or handle it? For now, just log.
    }

    // 9. Monitoring Phase
    system_registry_set_subtext("System Active");
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
                // Check registry for credentials (logic performed in system.c)
                // This handler just moves the state
                state_transition(SYSTEM_STATE_MONITORING);
            }
            break;
            
        case SYSTEM_EVENT_START_CONFIGURATION:
            state_transition(SYSTEM_STATE_CONFIGURING);
            break;

        case SYSTEM_EVENT_DEVICE_CONFIGURATED:
            if(is_system_at_state(SYSTEM_STATE_CONFIGURING)){
                state_transition(SYSTEM_STATE_MONITORING); // Re-init with new creds
            }
            break;

        case SYSTEM_EVENT_ELEVATOR_FAULT_DETECTED:
            // Handle specific faults...
            // start a task for sms
            // needs thinking and implementation later
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
