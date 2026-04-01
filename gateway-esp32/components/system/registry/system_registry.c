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

esp_err_t system_registry_init(void) {
    if (g_registry_mutex != NULL) return ESP_OK;

    g_registry_mutex = xSemaphoreCreateMutex();
    if (g_registry_mutex == NULL) return ESP_FAIL;

    memset(&g_registry, 0, sizeof(g_registry));
    g_registry.current_state = SYSTEM_STATE_IDLE;
    g_registry.has_wifi_creds = false;
    strncpy(g_registry.motion_state, "IDLE", sizeof(g_registry.motion_state) - 1);
    strncpy(g_registry.elevator_health, "GOOD", sizeof(g_registry.elevator_health) - 1);

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

void system_registry_update_wifi(int8_t level, int8_t rssi, bool connected) {
    if (LOCK_REG()) {
        g_registry.wifi_level = level;
        g_registry.wifi_rssi = rssi;
        g_registry.mqtt_connected = connected;
        UNLOCK_REG();
    }
}

void system_registry_update_cellular(int8_t level, int8_t csq, bool connected, const char* operator) {
    if (LOCK_REG()) {
        g_registry.cellular_level = level;
        g_registry.cellular_rssi = csq;
        g_registry.cellular_connected = connected;
        if (operator) {
            strncpy(g_registry.cellular_operator, operator, sizeof(g_registry.cellular_operator) - 1);
            g_registry.cellular_operator[sizeof(g_registry.cellular_operator) - 1] = '\0';
        }
        UNLOCK_REG();
    }
}

void system_registry_update_edge_status(bool connected) {
    if (LOCK_REG()) {
        g_registry.edge_node_connected = connected;
        UNLOCK_REG();
    }
}

void system_registry_update_motion(const char* motion) {
    if (LOCK_REG()) {
        if (motion) strncpy(g_registry.motion_state, motion, sizeof(g_registry.motion_state) - 1);
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
    if (!pkt || !LOCK_REG()) return;

    // Store raw and scaled values
    g_registry.raw_vibration = pkt->vibration;
    g_registry.scaled_vibration = (float)pkt->vibration / 100.0f;
    g_registry.ele_speed = pkt->speed;
    g_registry.raw_motion_state = pkt->motion_state;
    g_registry.raw_health_status = pkt->health_status;

    // Map Motion State
    switch (pkt->motion_state) {
        case MOTION_STATE_STATIONARY: strncpy(g_registry.motion_state, "IDLE", 15); break;
        case MOTION_STATE_MOVING_UP:  strncpy(g_registry.motion_state, "UP", 15); break;
        case MOTION_STATE_MOVING_DOWN: strncpy(g_registry.motion_state, "DOWN", 15); break;
        case MOTION_STATE_SHAKING:    strncpy(g_registry.motion_state, "SHAKE", 15); break;
        case MOTION_STATE_FREE_FALL:  strncpy(g_registry.motion_state, "FREEFALL", 15); break;
        default: strncpy(g_registry.motion_state, "UNKNOWN", 15); break;
    }

    // Map Health Status from Protocol
    switch (pkt->health_status) {
        case HEALTH_STABLE:
            strncpy(g_registry.elevator_health, "GOOD", 15);
            // If the Edge node says stable, we can clear our local fault flag
            // (Assuming the Edge node only sends STABLE if the fault is gone)
            g_registry.fault_active = false;
            if (g_registry.current_state == SYSTEM_STATE_ERROR) {
                g_registry.current_state = SYSTEM_STATE_MONITORING;
            }
            break;
        case HEALTH_WARNING:
            strncpy(g_registry.elevator_health, "WARNING", 15);
            break;
        case HEALTH_EMERGENCY:
            strncpy(g_registry.elevator_health, "CRITICAL", 15);
            g_registry.fault_active = true;
            g_registry.current_state = SYSTEM_STATE_ERROR;
            break;
        default:
            strncpy(g_registry.elevator_health, "UNKNOWN", 15);
            break;
    }

    UNLOCK_REG();
}

void system_registry_update_from_protocol_heartbeat(const edge_heartbeat_t *pkt) {
    if (!pkt || !LOCK_REG()) return;

    // Store raw values
    g_registry.raw_edge_health = pkt->edge_health;
    g_registry.raw_edge_state = pkt->edge_state;
    g_registry.raw_edge_uptime = pkt->uptime_sec;
    g_registry.raw_edge_error = pkt->error_code;

    g_registry.edge_node_connected = (pkt->edge_state == EDGE_STATE_RUNNING || pkt->edge_state == EDGE_STATE_INIT);
    
    if (pkt->edge_state == EDGE_STATE_ERROR) {
         strncpy(g_registry.sub_status, "EDGE NODE ERROR", 31);
    }
    UNLOCK_REG();
}

void system_registry_update_from_protocol_emergency(const ele_emergency_t *pkt) {
    if (!pkt || !LOCK_REG()) return;

    g_registry.last_fault_code = pkt->fault_code;
    g_registry.last_fault_severity = pkt->severity;
    g_registry.last_fault_value = pkt->fault_value;
    g_registry.last_fault_timestamp = pkt->timestamp;
    
    g_registry.fault_active = true;
    g_registry.current_state = SYSTEM_STATE_ERROR;
    
    // Map fault code to readable health string
    switch (pkt->fault_code) {
        case FAULT_SHAKE: strncpy(g_registry.elevator_health, "SHAKING", 15); break;
        case FAULT_FREEFALL: strncpy(g_registry.elevator_health, "FREEFALL", 15); break;
        case FAULT_EMERGENCY_STOP: strncpy(g_registry.elevator_health, "E-STOP", 15); break;
        default: strncpy(g_registry.elevator_health, "CRITICAL", 15); break;
    }

    UNLOCK_REG();
}
