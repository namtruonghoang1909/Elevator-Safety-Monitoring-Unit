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
 * @brief Map fault code to human readable message
 */
static const char* get_fault_msg(uint8_t code) {
    switch (code) {
        case FAULT_SHAKE:          return "EXCESSIVE SHAKE";
        case FAULT_FREEFALL:       return "FREE FALL DETECTED";
        case FAULT_EMERGENCY_STOP: return "EMERGENCY STOP";
        default:                   return "UNKNOWN FAULT";
    }
}

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
                snprintf(payload, sizeof(payload),
                    "{\"ele_fault_code\":%d,\"ele_fault_severity\":%d,\"ele_fault_value\":%d,\"ele_fault_msg\":\"%s\"}",
                    snap.last_fault_code, snap.last_fault_severity, snap.last_fault_value, get_fault_msg(snap.last_fault_code)
                );
                ESP_LOGW(TAG, "Sending Fault Telemetry: %s", payload);
                mqtt_manager_publish(MQTT_TOPIC, payload, 1, true);

            } else {
                // --- PERIODIC TELEMETRY (Unified Status) ---
                snprintf(payload, sizeof(payload), 
                    "{\"ele_motion\":\"%s\",\"ele_vibration\":%d,\"ele_speed\":%d,"
                    "\"edge_connected\":%s,\"edge_health\":%d,\"edge_uptime\":%lu,\"edge_error\":%d,"
                    "\"gw_wifi_rssi\":%d,\"gw_uptime\":%lu,\"gw_state\":%d}",
                    snap.motion_state, snap.raw_vibration, snap.ele_speed,
                    snap.edge_node_connected ? "true" : "false", snap.raw_edge_health, (unsigned long)snap.raw_edge_uptime, snap.raw_edge_error,
                    snap.wifi_level, (unsigned long)snap.uptime_sec, snap.current_state);

                ESP_LOGI(TAG, "Sending Unified Telemetry: %s", payload);

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
