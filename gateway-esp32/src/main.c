/**
 * @file main.c
 * @brief ESMU Gateway Entry Point
 * 
 * This file initializes the core hardware and services for the ESMU Gateway.
 */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "system_hw.h"
#include "system_registry.h"
#include "cellular_service.h"

static const char *TAG = "ESMU_GATEWAY";

void app_main(void) {
    ESP_LOGI(TAG, "Initializing ESMU Gateway...");

    // 1. Initialize System Status Registry
    if (system_registry_init() != ESP_OK) {
        ESP_LOGE(TAG, "Registry Initialization FAILED!");
        return;
    }

    // 2. Initialize System Hardware (UART, I2C, GPIOs, CAN)
    if (system_hw_init() != ESP_OK) {
        ESP_LOGE(TAG, "Hardware Initialization FAILED!");
        return;
    }

    // 3. Initialize & Start Cellular Service
    if (cellular_service_init() == ESP_OK) {
        cellular_service_start();
        ESP_LOGI(TAG, "Cellular Service started.");
    } else {
        ESP_LOGE(TAG, "Cellular Service Initialization FAILED!");
    }

    ESP_LOGI(TAG, "System ready. Entering main service loop...");

    while (1) {
        // Main loop can perform global monitoring or sleep
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
