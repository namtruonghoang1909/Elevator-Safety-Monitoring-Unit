/**
 * @file system.c
 * @brief Redesigned System Orchestrator Implementation
 */

#include "system.h"
#include "system_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "ESMU_SYS";

esp_err_t system_start(void) {
    ESP_LOGI(TAG, "Starting ESMU System Supervisor...");

    // 1. Stage 1 Init (Queue & Mutexes)
    esp_err_t ret = system_core_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "System core init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    system_report_event(SYSTEM_EVENT_BOOT);

    return ESP_OK;
}
