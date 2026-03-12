/**
 * @file task.c
 * @brief Thread-safe storage and registry synchronization for motion monitor
 */

#include <string.h>
#include "esp_log.h"
#include "task.h"
#include "system_registry.h"

esp_err_t mm_task_init(mm_task_ctx_t *ctx, const motion_monitor_config_t *cfg) {
    if (!ctx || !cfg) return ESP_ERR_INVALID_ARG;

    memset(ctx, 0, sizeof(mm_task_ctx_t));
    ctx->lock = xSemaphoreCreateMutex();
    if (!ctx->lock) return ESP_ERR_NO_MEM;

    ctx->metrics.state = MOTION_STATE_STATIONARY;
    ctx->metrics.balance = BALANCE_STATE_LEVEL;

    return ESP_OK;
}

esp_err_t mm_task_update_metrics(mm_task_ctx_t *ctx, const motion_metrics_t *metrics) {
    if (!ctx || !metrics || !ctx->lock) return ESP_ERR_INVALID_ARG;

    xSemaphoreTake(ctx->lock, portMAX_DELAY);
    
    // Copy remote metrics to local storage
    memcpy(&ctx->metrics, metrics, sizeof(motion_metrics_t));
    
    // Translate internal states to readable strings for the Registry/UI
    const char* m_str = "IDLE";
    switch (ctx->metrics.state) {
        case MOTION_STATE_MOVING_UP:         m_str = "UP"; break;
        case MOTION_STATE_MOVING_DOWN:       m_str = "DOWN"; break;
        case MOTION_STATE_DECELERATING_UP:   m_str = "BRAKE UP"; break;
        case MOTION_STATE_DECELERATING_DOWN: m_str = "BRAKE DN"; break;
        case MOTION_STATE_ACCELERATING:      m_str = "ACCEL"; break;
        default: m_str = "IDLE"; break;
    }

    const char* b_str = "STABLE";
    switch (ctx->metrics.balance) {
        case BALANCE_STATE_TILT_LEFT:     b_str = "TILT LEFT"; break;
        case BALANCE_STATE_TILT_RIGHT:    b_str = "TILT RIGHT"; break;
        case BALANCE_STATE_TILT_FORWARD:  b_str = "TILT FWD"; break;
        case BALANCE_STATE_TILT_BACKWARD: b_str = "TILT BWD"; break;
        default: b_str = "STABLE"; break;
    }

    system_registry_update_motion(m_str, b_str);
    
    xSemaphoreGive(ctx->lock);
    return ESP_OK;
}
