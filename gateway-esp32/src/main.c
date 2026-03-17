/**
 * @file main.c
 * @brief ESMU Gateway Main - Efficient Grouped MQTT Publisher
 * 
 * This application initializes the ESMU system and periodically publishes 
 * grouped protocol data (Health, Fault, Heartbeat) to the MQTT broker.
 * Each protocol packet is sent as a single MQTT publish.
 */

#include "nvs_flash.h"
#include "esp_log.h"
#include "connectivity_manager.h"
#include "system_config.h"
#include "esmu_protocol.h"
#include "display_service.h"
#include "system_registry.h"
#include "i2c_platform.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "telemetry_service.h"

static const char *TAG = "MAIN";

/**
 * @brief Task to periodically update registry with mock data.
 * The real telemetry_service will pick this up and publish to MQTT.
 */
static void mock_data_provider_task(void *pvParameters) {
    uint32_t uptime_sim = 0;

    ESP_LOGI(TAG, "Mock Data Provider Started");

    while (1) {
        // 1. Mock Elevator Health
        ele_health_t health = {
            .avg_tilt = 120,
            .max_tilt = 450,
            .balance = BALANCE_STATE_TILT_LEFT,
            .health_score = 95
        };
        system_registry_update_from_protocol_health(&health);

        // 2. Mock Edge Heartbeat
        edge_heartbeat_t hb = {
            .edge_health = EDGE_HEALTH_OK,
            .edge_state = EDGE_STATE_RUNNING,
            .uptime_sec = uptime_sim++
        };
        system_registry_update_from_protocol_heartbeat(&hb);

        // 3. Mock Elevator Emergency (every 60s)
        if (uptime_sim % 12 == 0) {
            ele_emergency_t fault = {
                .fault_code = FAULT_OVERTILT,
                .severity = 5,
                .fault_value = 1500
            };
            system_registry_update_from_protocol_emergency(&fault);
            system_registry_set_subtext("MOCK EMERGENCY ACTIVE");
            
            // Trigger immediate MQTT publish for fault
            telemetry_service_force_publish();
            ESP_LOGW(TAG, "Mock Fault Injected & Force Publish Triggered");
        } else if (uptime_sim % 12 == 3) {
            // Clear fault after a bit
            system_registry_set_state(SYSTEM_STATE_MONITORING);
            system_registry_set_subtext("Gateway Active");
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // Update every 5s to match telemetry interval
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "ESMU Gateway Booting...");

    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    // 2. Initialize Hardware & System Registry
    uint8_t bus_id = 0;
    ESP_ERROR_CHECK(i2c_bus_init(&bus_id, (gpio_num_t)I2C_SDA_PIN, (gpio_num_t)I2C_SCL_PIN));
    ESP_ERROR_CHECK(system_registry_init());
    ESP_ERROR_CHECK(display_service_init());

    // 3. Initialize Connectivity
    connectivity_config_t conn_cfg = {
        .wifi_config = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .auto_reconnect = true
        },
        .mqtt_config = {
            .broker_uri = BROKER_URI,
            .client_id = CLIENT_ID,
            .username = CLIENT_USERNAME,
            .password = CLIENT_PASSWORD,
            .port = 1883
        }
    };
    ESP_ERROR_CHECK(connectivity_manager_init(&conn_cfg));
    ESP_ERROR_CHECK(connectivity_manager_start());

    // 4. Initialize Telemetry Service (Handles MQTT publishing)
    ESP_ERROR_CHECK(telemetry_service_init());

    // 5. Start Mock Data Task
    xTaskCreate(mock_data_provider_task, "data_mock", 4096, NULL, 5, NULL);

    // 6. Set System State
    system_registry_set_state(SYSTEM_STATE_MONITORING);

    ESP_LOGI(TAG, "ESMU Gateway Ready - Using Telemetry Service");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
