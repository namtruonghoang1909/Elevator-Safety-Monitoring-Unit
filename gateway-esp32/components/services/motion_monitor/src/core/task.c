/**
 * @file task.c
 * @brief FreeRTOS task and synchronization for motion monitor
 */

#include <string.h>
#include "esp_log.h"
#include "task.h"
#include "system_registry.h"

static const char *TAG = "MOTION_TASK";

static void motion_monitor_task(void *arg) {
    mm_task_ctx_t *ctx = (mm_task_ctx_t *)arg;
    TickType_t last_wake_time = xTaskGetTickCount();
    mpu6050_scaled_data_t raw;

    while (1) {
        if (mpu6050_read_scaled(ctx->mpu_id, &raw) == ESP_OK) {
            mm_task_process_sample(ctx, &raw);
        }
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10));
    }
}

esp_err_t mm_task_init(mm_task_ctx_t *ctx, const motion_monitor_config_t *cfg) {
    if (!ctx || !cfg) return ESP_ERR_INVALID_ARG;

    memset(ctx, 0, sizeof(mm_task_ctx_t));
    ctx->mpu_id = cfg->mpu_dev_id;
    ctx->lock = xSemaphoreCreateMutex();
    if (!ctx->lock) return ESP_ERR_NO_MEM;

    float alpha = (cfg->filter_alpha > 0) ? cfg->filter_alpha : 0.2f;
    mm_processor_init(&ctx->processor, alpha);
    
    ctx->metrics.state = MOTION_STATE_STATIONARY;
    ctx->metrics.balance = BALANCE_STATE_LEVEL;

    return ESP_OK;
}

esp_err_t mm_task_start(mm_task_ctx_t *ctx, const motion_monitor_config_t *cfg) {
    if (!ctx || !cfg) return ESP_ERR_INVALID_ARG;
    
    if (ctx->mpu_id != 0xFF) {
        uint32_t stack = (cfg->task_stack > 0) ? cfg->task_stack : 4096;
        uint32_t prio = (cfg->task_priority > 0) ? cfg->task_priority : configMAX_PRIORITIES - 2;
        
        BaseType_t ret = xTaskCreate(motion_monitor_task, "motion_task", stack, ctx, prio, &ctx->task_handle);
        ESP_LOGI(TAG, "Motion monitor task started!");
        if (ret != pdPASS) return ESP_FAIL;
    }
    
    return ESP_OK;
}

void mm_task_process_sample(mm_task_ctx_t *ctx, const mpu6050_scaled_data_t *raw) {
    if (!ctx || !raw || !ctx->lock) return;

    xSemaphoreTake(ctx->lock, portMAX_DELAY);
    
    mm_processor_run(&ctx->processor, raw, &ctx->metrics);
    ctx->metrics.last_update = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Translate internal states to readable strings for the Registry/UI
    const char* m_str = "IDLE";
    if (ctx->metrics.state == MOTION_STATE_MOVING_UP) m_str = "UP";
    else if (ctx->metrics.state == MOTION_STATE_MOVING_DOWN) m_str = "DOWN";

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
}
