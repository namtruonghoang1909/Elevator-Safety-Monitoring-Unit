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
    strncpy(g_registry.balance_state, "STABLE", sizeof(g_registry.balance_state) - 1);
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

void system_registry_update_wifi(int8_t level, bool connected) {
    if (LOCK_REG()) {
        g_registry.wifi_level = level;
        g_registry.mqtt_connected = connected;
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
