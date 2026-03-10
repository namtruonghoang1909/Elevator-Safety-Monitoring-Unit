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
    ESP_ERROR_CHECK(system_core_init());

    system_report_event(SYSTEM_EVENT_BOOT);

    return ESP_OK;

}
