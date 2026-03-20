/**
 * @file system_registry.c
 * @brief Thread-safe System Registry (Whiteboard) implementation
 */

#include "system_registry.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <string.h>

// ─────────────────────────────────────────────
// Private variables
// ─────────────────────────────────────────────

static system_registry_t registry = {0};
static SemaphoreHandle_t registry_mutex = NULL;
static StaticSemaphore_t registry_mutex_buffer;

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

bool system_registry_init(void) {
    if (registry_mutex == NULL) {
        registry_mutex = xSemaphoreCreateMutexStatic(&registry_mutex_buffer);
    }
    if (registry_mutex == NULL) return false;

    memset(&registry, 0, sizeof(registry));
    registry.state = NODE_STATE_INIT;
    registry.metrics.health_score = 100;
    registry.last_update_tick = xTaskGetTickCount();

    return true;
}

bool system_registry_read(system_registry_t *out_data) {
    if (registry_mutex == NULL) return false;

    if (xSemaphoreTake(registry_mutex, portMAX_DELAY) == pdTRUE) {
        memcpy(out_data, &registry, sizeof(registry));
        xSemaphoreGive(registry_mutex);
        return true;
    }
    return false;
}

bool system_registry_write(const system_registry_t *data) {
    if (registry_mutex == NULL) return false;

    if (xSemaphoreTake(registry_mutex, portMAX_DELAY) == pdTRUE) {
        memcpy(&registry, data, sizeof(registry));
        registry.last_update_tick = xTaskGetTickCount();
        xSemaphoreGive(registry_mutex);
        return true;
    }
    return false;
}

void system_registry_set_state(node_state_t state) {
    if (registry_mutex == NULL) return;

    if (xSemaphoreTake(registry_mutex, portMAX_DELAY) == pdTRUE) {
        registry.state = state;
        registry.last_update_tick = xTaskGetTickCount();
        xSemaphoreGive(registry_mutex);
    }
}

node_state_t system_registry_get_state(void) {
    node_state_t state = NODE_STATE_ERROR;
    if (registry_mutex == NULL) return NODE_STATE_ERROR;

    if (xSemaphoreTake(registry_mutex, portMAX_DELAY) == pdTRUE) {
        state = registry.state;
        xSemaphoreGive(registry_mutex);
    }
    return state;
}
