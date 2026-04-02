/**
 * @file heartbeat.c
 * @brief Visual Heartbeat Service Implementation
 */

#include "heartbeat.h"
#include "system_config.h"
#include "system_registry.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "HEARTBEAT";

// ─────────────────────────────────────────────
// Private variables
// ─────────────────────────────────────────────
static TaskHandle_t s_heartbeat_task_handle = NULL;
static bool s_is_initialized = false;

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

/**
 * @brief Main heartbeat task loop
 */
static void heartbeat_task(void *pvParameters) {
    uint32_t uptime_sec = 0;
    bool led_state = false;

    ESP_LOGI(TAG, "Heartbeat task started on GPIO %d", CONFIG_PIN_STATUS_LED);

    while (1) {
        // Update Registry Uptime
        system_registry_update_uptime(uptime_sec++);

        // Toggle LED
        led_state = !led_state;
        gpio_set_level((gpio_num_t)CONFIG_PIN_STATUS_LED, led_state);

        // 1Hz blink (1000ms period)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

esp_err_t heartbeat_init(void) {
    if (s_is_initialized) {
        return ESP_OK;
    }

    // Initialize GPIO
    gpio_reset_pin((gpio_num_t)CONFIG_PIN_STATUS_LED);
    gpio_set_direction((gpio_num_t)CONFIG_PIN_STATUS_LED, GPIO_MODE_OUTPUT);

    // Create Task
    BaseType_t ret = xTaskCreate(heartbeat_task, "heartbeat", 2048, NULL, tskIDLE_PRIORITY + 1, &s_heartbeat_task_handle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create heartbeat task");
        return ESP_FAIL;
    }

    s_is_initialized = true;
    return ESP_OK;
}
