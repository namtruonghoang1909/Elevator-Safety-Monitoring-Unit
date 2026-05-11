/**
 * @file main.c
 * @brief ESMU Gateway Entry Point
 */

#include "system.h"
#include "esp_log.h"
#include "ota_manager.h"

static const char *TAG = "MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "ESMU Gateway starting...");

    // Initialize OTA Manager
    ota_manager_init();

    // Start the System Orchestrator
    system_start();
}
