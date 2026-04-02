/**
 * @file system_boot.c
 * @brief Service Boot Orchestrator Implementation
 */

#include "system_boot.h"
#include "system_config.h"
#include "system_registry.h"
#include "system_event.h"
#include "connectivity_manager.h"
#include "motion_proxy.h"
#include "telemetry_service.h"
#include "cellular_service.h"
#include "can_platform.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "SYS_BOOT";

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

static void boot_task(void *pvParameters) {
    ESP_LOGI(TAG, "Starting ESMU Boot Sequence...");
    esp_err_t ret = ESP_OK;

    // 1. Display Service
    system_registry_set_subtext("Display Init...");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display service failed: %s", esp_err_to_name(ret));
        system_registry_set_subtext("ERR: DISPLAY FAIL");
        // We might continue or fail here depending on criticality
    }
    system_registry_set_subtext("Booting ESMU...");
    vTaskDelay(pdMS_TO_TICKS(500));

    // 2. Connectivity Manager
    system_registry_set_subtext("Connectivity Init...");
    connectivity_config_t conn_cfg = {
        .wifi_config = { .ssid = NULL, .password = NULL, .auto_reconnect = true },
        .mqtt_config = { 
            .broker_uri = BROKER_URI, 
            .client_id = CLIENT_ID,
            .username = CLIENT_USERNAME,
            .password = CLIENT_PASSWORD,
            .port = 1883,
        },
    };
    ret = connectivity_manager_init(&conn_cfg);
    if (ret == ESP_OK) {
        connectivity_manager_start();
    } else {
        ESP_LOGE(TAG, "Connectivity failed: %s", esp_err_to_name(ret));
    }

    // 3. CAN Start
    system_registry_set_subtext("Starting CAN...");
    platform_can_start();

    // 4. Motion Proxy
    system_registry_set_subtext("Waiting for Edge...");
    ret = motion_proxy_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Motion proxy failed: %s", esp_err_to_name(ret));
    }

    // 5. Telemetry
    ret = telemetry_service_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Telemetry service failed: %s", esp_err_to_name(ret));
    }

    // 6. Cellular
    system_registry_set_subtext("Starting 4G...");
    if (cellular_service_init() == ESP_OK) {
        cellular_service_start();
    } else {
        ESP_LOGE(TAG, "Cellular service failed");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    system_registry_set_subtext("System Ready");
    
    // Notify Controller that boot is complete
    system_report_event(SYSTEM_EVENT_INITIALIZED);
    
    ESP_LOGI(TAG, "Boot Sequence Complete");
    vTaskDelete(NULL);
}

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

esp_err_t system_boot_begin(void) {
    BaseType_t ret = xTaskCreate(boot_task, "boot_task", 4096, NULL, 5, NULL);
    return (ret == pdPASS) ? ESP_OK : ESP_FAIL;
}
