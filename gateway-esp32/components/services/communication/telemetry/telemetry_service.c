/**
 * @file telemetry_service.c
 * @brief MQTT Telemetry implementation for ESMU Gateway
 */

#include "telemetry_service.h"
#include "mqtt_manager.h"
#include "system_registry.h"
#include "system_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "TELEMETRY";
static SemaphoreHandle_t s_force_publish_sem = NULL;

/**
 * @brief Telemetry publishing task
 * Publishes every 2 seconds or immediately on force event.
 */
static void telemetry_task(void *pvParameters) {
    system_status_registry_t snap;
    char payload[256];
    
    ESP_LOGI(TAG, "Telemetry service task started (5s interval)");

    while (1) {
        // Wait for 5 seconds OR an immediate event (fault)
        BaseType_t triggered = xSemaphoreTake(s_force_publish_sem, pdMS_TO_TICKS(5000));

        if (mqtt_manager_is_connected()) {
            system_registry_get_snapshot(&snap);

            if (triggered == pdTRUE && snap.fault_active) {
                // --- IMMEDIATE FAULT REPORTING ---
                ESP_LOGW(TAG, "Immediate Fault Telemetry Triggered!");
                snprintf(payload, sizeof(payload),
                    "{\"fault_code\":%d,\"severity\":%d,\"fault_val\":%d,\"msg\":\"%s\"}",
                    snap.last_fault_code, snap.last_fault_severity, snap.last_fault_value, snap.sub_status
                );
                mqtt_manager_publish(MQTT_TOPIC, payload, 1, true);
            } else {
                // --- PERIODIC TELEMETRY (Health + Heartbeat) ---
                // Send Health Data
                snprintf(payload, sizeof(payload), 
                    "{\"type\":\"health\",\"avg_tilt\":%d,\"max_tilt\":%d,\"balance\":%d,\"score\":%d}",
                    snap.raw_avg_tilt, snap.raw_max_tilt, snap.raw_balance, snap.raw_health_score);
                mqtt_manager_publish(MQTT_TOPIC, payload, 0, false);

                // Send Heartbeat/Node Status
                snprintf(payload, sizeof(payload),
                    "{\"type\":\"node\",\"edge_health\":%d,\"edge_state\":%d,\"edge_uptime\":%lu,\"wifi_rssi\":%d}",
                    snap.raw_edge_health, snap.raw_edge_state, (unsigned long)snap.raw_edge_uptime, snap.wifi_level);
                mqtt_manager_publish(MQTT_TOPIC, payload, 0, false);
            }
        }
    }
}

esp_err_t telemetry_service_init(void) {
    s_force_publish_sem = xSemaphoreCreateBinary();
    if (!s_force_publish_sem) return ESP_ERR_NO_MEM;

    BaseType_t ret = xTaskCreate(telemetry_task, "tele_pub_task", 4096, NULL, 4, NULL);
    if (ret != pdPASS) return ESP_FAIL;

    return ESP_OK;
}

void telemetry_service_force_publish(void) {
    if (s_force_publish_sem) {
        xSemaphoreGive(s_force_publish_sem);
    }
}
