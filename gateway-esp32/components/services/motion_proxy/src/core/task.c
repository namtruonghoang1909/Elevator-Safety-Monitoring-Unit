/**
 * @file task.c
 * @brief CAN listener and registry bridge for Motion Proxy
 */

#include <string.h>
#include "esp_log.h"
#include "task.h"
#include "can_bsp.h"
#include "system_registry.h"
#include "esmu_protocol.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MOTION_PROXY";
static mp_task_ctx_t s_ctx;

/**
 * @brief Background task that waits for CAN messages and updates the registry
 */
static void can_listener_task(void *pvParameters) {
    uint32_t id;
    uint8_t data[8];
    uint8_t len;

    ESP_LOGI(TAG, "CAN Proxy listener started");

    while (1) {
        // Block until a CAN message arrives
        esp_err_t err = can_bsp_receive(&id, data, &len, portMAX_DELAY);

        if (err == ESP_OK && len == 8) {
            xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
            
            switch (id) {
                case CAN_ID_ELE_HEALTH: {
                    ele_health_t pkt;
                    memcpy(&pkt, data, 8);
                    ESP_LOGI(TAG, "HEALTH: Tilt Avg %d Max %d Score %d", pkt.avg_tilt, pkt.max_tilt, pkt.health_score);
                    system_registry_update_from_protocol_health(&pkt);
                    break;
                }
                case CAN_ID_ELE_EMERGENCY: {
                    ele_emergency_t pkt;
                    memcpy(&pkt, data, 8);
                    ESP_LOGW(TAG, "!! EMERGENCY RECEIVED !! Fault Code: %d Severity: %d", pkt.fault_code, pkt.severity);
                    system_registry_update_from_protocol_emergency(&pkt);
                    // system_registry_set_subtext("FAULT DETECTED ON EDGE");
                    break;
                }
                case CAN_ID_EDGE_HEALTH: {
                    edge_heartbeat_t hb;
                    memcpy(&hb, data, 8);
                    s_ctx.last_heartbeat_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    ESP_LOGI(TAG, "HEARTBEAT: State %d Uptime %lu sec", hb.edge_state, hb.uptime_sec);
                    system_registry_update_from_protocol_heartbeat(&hb);
                    break;
                }
                case CAN_ID_TEST_SIGNAL: {
                    test_packet_t pkt;
                    memcpy(&pkt, data, sizeof(pkt));
                    
                    if (pkt.magic_byte == 0xA5) {
                        char buf[32];
                        snprintf(buf, sizeof(buf), "UPTIME: %lu", pkt.uptime);
                        system_registry_set_subtext(buf);
                        ESP_LOGI(TAG, "CAN Signal Received - Uptime: %lu", pkt.uptime);
                    } else {
                        ESP_LOGW(TAG, "CAN Signal Received but Magic Byte Mismatch: 0x%02X", pkt.magic_byte);
                    }
                    break;
                }
                default:
                    break;
            }

            xSemaphoreGive(s_ctx.lock);
        }
    }
}

/**
 * @brief Background task to monitor connectivity
 */
static void watchdog_task(void *pvParameters) {
    while (1) {
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        uint32_t last;

        xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
        last = s_ctx.last_heartbeat_ms;
        xSemaphoreGive(s_ctx.lock);

        // If no heartbeat for 3 seconds, mark as offline
        if (last != 0 && (now - last) > 3000) {
            system_registry_update_edge_status(false);
            system_registry_set_subtext("COMMUNICATION LOST");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

esp_err_t mp_task_init(void) {
    memset(&s_ctx, 0, sizeof(mp_task_ctx_t));
    s_ctx.lock = xSemaphoreCreateMutex();
    if (!s_ctx.lock) return ESP_ERR_NO_MEM;

    xTaskCreate(can_listener_task, "proxy_can_rx", 4096, NULL, 10, NULL);
    xTaskCreate(watchdog_task, "proxy_watchdog", 2048, NULL, 5, NULL);

    return ESP_OK;
}

bool mp_task_is_alive(uint32_t timeout_ms) {
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    bool alive = false;

    if (xSemaphoreTake(s_ctx.lock, pdMS_TO_TICKS(10)) == pdTRUE) {
        alive = (now - s_ctx.last_heartbeat_ms) < timeout_ms;
        xSemaphoreGive(s_ctx.lock);
    }

    return alive;
}
