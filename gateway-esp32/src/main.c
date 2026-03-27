/**
 * @file main.c
 * @brief ESMU CAN Playground - Minimal Monitor
 */

#include "esp_log.h"
#include "system.h"
#include <stdio.h>

static const char *TAG = "MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "ESMU Starting...");

    // 0. Start System Controller and Services
    ESP_ERROR_CHECK(system_start());

    ESP_LOGI(TAG, "Monitor Ready. System managed by Supervisor Task.");
}
