#include "system.h"
#include "system_event.h"
#include "system_error.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "SYSTEM";

void system_start(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize system controller (queues, etc.)
    // system_controller_init();
    
    ESP_LOGI(TAG, "System starting...");
    
    // Report boot event to start the FSM
    // system_report_event(SYSTEM_EVENT_BOOT);
}


