/**
 * @file telemetry_service.c
 * @brief MQTT Telemetry implementation with Independent Timers
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

#define INTERVAL_ELE_HEALTH_MS  3000
#define INTERVAL_EDGE_HEALTH_MS 5000
#define INTERVAL_GW_HEALTH_MS   10000

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
 * High-frequency check (100ms) with independent timers for each telemetry type.
 */
static void telemetry_task(void *pvParameters) {
    system_status_registry_t snap;
    char payload[256];
    
    uint32_t last_ele_ms = 0;
    uint32_t last_edge_ms = 0;
    uint32_t last_gw_ms = 0;

    ESP_LOGI(TAG, "Telemetry service task started");

    while (1) {
        // Wait for a force event (Emergency) OR a short tick (100ms)
        BaseType_t triggered = xSemaphoreTake(s_force_publish_sem, pdMS_TO_TICKS(100));
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (mqtt_manager_is_connected()) {
            system_registry_get_snapshot(&snap);

            // --- 1. EMERGENCY (Immediate on Trigger) ---
            if (triggered == pdTRUE && snap.fault_active) {
                snprintf(payload, sizeof(payload),
                    "{\"ele_fault_active\":true,\"ele_fault_code\":%d,\"ele_fault_severity\":%d,\"ele_fault_value\":%d,\"ele_fault_msg\":\"%s\"}",
                    snap.last_fault_code, snap.last_fault_severity, snap.last_fault_value, get_fault_msg(snap.last_fault_code)
                );
                mqtt_manager_publish(MQTT_TOPIC, payload, 1, false);
                ESP_LOGW(TAG, "Emergency Telemetry Sent");
            }

            // --- 2. ELEVATOR HEALTH (Every 3s) ---
            if (now - last_ele_ms >= INTERVAL_ELE_HEALTH_MS) {
                snprintf(payload, sizeof(payload), 
                    "{\"ele_motion\":\"%s\",\"ele_vibration\":%.2f,\"ele_speed\":%d,\"ele_health\":%d}",
                    snap.motion_state, snap.scaled_vibration, snap.ele_speed, snap.raw_health_status);
                
                mqtt_manager_publish(MQTT_TOPIC, payload, 0, false);
                last_ele_ms = now;
                ESP_LOGI(TAG, "Ele Health Telemetry Sent");
            }

            // --- 3. EDGE NODE HEALTH (Every 5s) ---
            if (now - last_edge_ms >= INTERVAL_EDGE_HEALTH_MS) {
                snprintf(payload, sizeof(payload), 
                    "{\"edge_connected\":%s,\"edge_health\":%d,\"edge_uptime\":%lu,\"edge_error\":%d}",
                    snap.edge_node_connected ? "true" : "false", snap.raw_edge_health, (unsigned long)snap.raw_edge_uptime, snap.raw_edge_error);

                mqtt_manager_publish(MQTT_TOPIC, payload, 0, false);
                last_edge_ms = now;
                ESP_LOGI(TAG, "Edge Health Telemetry Sent");
            }

            // --- 4. GATEWAY STATUS (Every 10s) ---
            if (now - last_gw_ms >= INTERVAL_GW_HEALTH_MS) {
                snprintf(payload, sizeof(payload), 
                    "{\"gw_wifi_rssi\":%d,\"gw_uptime\":%lu,\"gw_state\":%d}",
                    snap.wifi_rssi, (unsigned long)snap.uptime_sec, snap.current_state);
                
                mqtt_manager_publish(MQTT_TOPIC, payload, 0, false);
                last_gw_ms = now;
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
