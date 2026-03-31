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
#include "sim_a7680c.h"

static const char *TAG = "ESMU_GATEWAY";

void app_main(void) {
    ESP_LOGI(TAG, "Initializing ESMU Gateway...");

    // 1. Initialize System Hardware (UART, I2C, GPIOs)
    if (system_hw_init() != ESP_OK) {
        ESP_LOGE(TAG, "Hardware Initialization FAILED!");
        return;
    }

    // 2. Initialize SIM A7680C Driver
    // Note: Hardware reset and sync are performed during init
    sim_a7680c_hw_reset();
    if (sim_a7680c_init() != ESP_OK) {
        ESP_LOGW(TAG, "SIM Module not responding or not present.");
    } else {
        ESP_LOGI(TAG, "SIM Driver initialized successfully.");
    }

    ESP_LOGI(TAG, "System ready. Entering main service loop...");

    while (1) {
        // Placeholder for future Service Orchestration (Connectivity Manager, etc.)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
