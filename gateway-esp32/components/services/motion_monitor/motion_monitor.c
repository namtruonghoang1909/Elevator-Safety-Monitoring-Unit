/**
 * @file motion_monitor.c
 * @brief Motion Monitor service (Remote Proxy version for Distributed ESMU)
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "motion_monitor.h"
#include "src/core/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MOTION_MONITOR";

static mm_task_ctx_t s_ctx;

esp_err_t motion_monitor_init(const motion_monitor_config_t *cfg) {
    esp_err_t err = mm_task_init(&s_ctx, cfg);
    if (err != ESP_OK) return err;

    ESP_LOGI(TAG, "Service initialized in Remote Proxy mode");
    return ESP_OK;
}

esp_err_t motion_monitor_update(const motion_metrics_t *metrics) {
    return mm_task_update_metrics(&s_ctx, metrics);
}

esp_err_t motion_monitor_get_metrics(motion_metrics_t *out) {
    if (!out || !s_ctx.lock) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
    memcpy(out, &s_ctx.metrics, sizeof(motion_metrics_t));
    xSemaphoreGive(s_ctx.lock);
    return ESP_OK;
}

motion_state_t motion_monitor_get_motion(void) {
    motion_state_t state;
    if (!s_ctx.lock) return MOTION_STATE_STATIONARY;
    xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
    state = s_ctx.metrics.state;
    xSemaphoreGive(s_ctx.lock);
    return state;
}

balance_state_t motion_monitor_get_equilibrium(void) {
    balance_state_t balance;
    if (!s_ctx.lock) return BALANCE_STATE_LEVEL;
    xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
    balance = s_ctx.metrics.balance;
    xSemaphoreGive(s_ctx.lock);
    return balance;
}

bool motion_monitor_is_alive(uint32_t timeout_ms) {
    if (!s_ctx.lock) return false;
    
    xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
    uint32_t last = s_ctx.metrics.last_update;
    xSemaphoreGive(s_ctx.lock);

    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    return (now - last) < timeout_ms;
}
