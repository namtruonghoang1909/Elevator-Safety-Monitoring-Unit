/**
 * @file system_registry.c
 * @brief Thread-safe status registry implementation
 */

#include "system_registry.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include <string.h>

// static const char *TAG = "SYS_REG";

static system_status_registry_t g_registry;
static SemaphoreHandle_t g_registry_mutex = NULL;

#define LOCK_REG()   (xSemaphoreTake(g_registry_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
#define UNLOCK_REG() (xSemaphoreGive(g_registry_mutex))

/**
 * @brief Background task to increment and update uptime
 */
static void uptime_task(void *pvParameters) {
    uint32_t uptime_sec = 0;
    while (1) {
        if (LOCK_REG()) {
            g_registry.uptime_sec = uptime_sec++;
            UNLOCK_REG();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

esp_err_t system_registry_init(void) {
    if (g_registry_mutex != NULL) return ESP_OK;

    g_registry_mutex = xSemaphoreCreateMutex();
    if (g_registry_mutex == NULL) return ESP_FAIL;

    memset(&g_registry, 0, sizeof(g_registry));
    g_registry.current_state = SYSTEM_STATE_IDLE;
    g_registry.has_wifi_creds = false;
    strncpy(g_registry.motion_state, "IDLE", sizeof(g_registry.motion_state) - 1);
    strncpy(g_registry.balance_state, "STABLE", sizeof(g_registry.balance_state) - 1);
    strncpy(g_registry.elevator_health, "GOOD", sizeof(g_registry.elevator_health) - 1);

    // Create background uptime task
    xTaskCreate(uptime_task, "uptime_task", 2048, NULL, 1, NULL);

    return ESP_OK;
}

void system_registry_set_state(system_state_id_t state) {
    if (LOCK_REG()) {
        g_registry.current_state = state;
        g_registry.fault_active = (state == SYSTEM_STATE_ERROR);
        UNLOCK_REG();
    }
}

void system_registry_set_subtext(const char* text) {
    if (LOCK_REG()) {
        if (text) {
            strncpy(g_registry.sub_status, text, sizeof(g_registry.sub_status) - 1);
            g_registry.sub_status[sizeof(g_registry.sub_status) - 1] = '\0';
        }
        UNLOCK_REG();
    }
}

void system_registry_update_wifi(int8_t level, bool connected) {
    if (LOCK_REG()) {
        g_registry.wifi_level = level;
        g_registry.mqtt_connected = connected;
        UNLOCK_REG();
    }
}

void system_registry_update_edge_status(bool connected) {
    if (LOCK_REG()) {
        g_registry.edge_node_connected = connected;
        UNLOCK_REG();
    }
}

void system_registry_update_motion(const char* motion, const char* balance) {
    if (LOCK_REG()) {
        if (motion) strncpy(g_registry.motion_state, motion, sizeof(g_registry.motion_state) - 1);
        if (balance) strncpy(g_registry.balance_state, balance, sizeof(g_registry.balance_state) - 1);
        UNLOCK_REG();
    }
}

void system_registry_update_health(const char* health) {
    if (LOCK_REG()) {
        if (health) strncpy(g_registry.elevator_health, health, sizeof(g_registry.elevator_health) - 1);
        UNLOCK_REG();
    }
}

void system_registry_set_wifi_credentials(bool set) {
    if (LOCK_REG()) {
        g_registry.has_wifi_creds = set;
        UNLOCK_REG();
    }
}

void system_registry_update_uptime(uint32_t uptime_sec) {
    if (LOCK_REG()) {
        g_registry.uptime_sec = uptime_sec;
        UNLOCK_REG();
    }
}

void system_registry_get_snapshot(system_status_registry_t *out) {
    if (out == NULL) return;
    if (LOCK_REG()) {
        memcpy(out, &g_registry, sizeof(system_status_registry_t));
        UNLOCK_REG();
    }
}

void system_registry_update_from_protocol_health(const ele_health_t *pkt) {
    if (!pkt) return;
    if (LOCK_REG()) {
        // Store raw values
        g_registry.raw_avg_tilt = pkt->avg_tilt;
        g_registry.raw_max_tilt = pkt->max_tilt;
        g_registry.raw_balance = pkt->balance;
        g_registry.raw_health_score = pkt->health_score;

        // 1. Map Balance
        switch (pkt->balance) {
            case BALANCE_STATE_LEVEL: strncpy(g_registry.balance_state, "STABLE", 15); break;
            case BALANCE_STATE_TILT_LEFT: strncpy(g_registry.balance_state, "TILT LEFT", 15); break;
            case BALANCE_STATE_TILT_RIGHT: strncpy(g_registry.balance_state, "TILT RIGHT", 15); break;
            case BALANCE_STATE_TILT_FORWARD: strncpy(g_registry.balance_state, "TILT FWD", 15); break;
            case BALANCE_STATE_TILT_BACKWARD: strncpy(g_registry.balance_state, "TILT BWD", 15); break;
            default: strncpy(g_registry.balance_state, "UNKNOWN", 15); break;
        }

        // 2. Map Health Score
        if (pkt->health_score > 90) strncpy(g_registry.elevator_health, "GOOD", 15);
        else if (pkt->health_score > 60) strncpy(g_registry.elevator_health, "SICK", 15);
        else strncpy(g_registry.elevator_health, "CRITICAL", 15);

        UNLOCK_REG();
    }
}

void system_registry_update_from_protocol_heartbeat(const edge_heartbeat_t *pkt) {
    if (!pkt) return;
    if (LOCK_REG()) {
        // Store raw values
        g_registry.raw_edge_health = pkt->edge_health;
        g_registry.raw_edge_state = pkt->edge_state;
        g_registry.raw_edge_uptime = pkt->uptime_sec;

        g_registry.edge_node_connected = (pkt->edge_state == EDGE_STATE_RUNNING || pkt->edge_state == EDGE_STATE_INIT);
        
        if (pkt->edge_state == EDGE_STATE_ERROR) {
             strncpy(g_registry.sub_status, "EDGE NODE ERROR", 31);
        }
        UNLOCK_REG();
    }
}

void system_registry_update_from_protocol_emergency(const ele_emergency_t *pkt) {
    if (!pkt) return;
    if (LOCK_REG()) {
        g_registry.last_fault_code = pkt->fault_code;
        g_registry.last_fault_severity = pkt->severity;
        g_registry.last_fault_value = pkt->fault_value;
        g_registry.fault_active = true;
        g_registry.current_state = SYSTEM_STATE_ERROR;
        UNLOCK_REG();
    }
}
