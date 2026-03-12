/**
 * @file task.h
 * @brief Thread-safe storage and registry synchronization for motion monitor
 */

#pragma once

#include "motion_monitor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @brief Motion monitor task context
 */
typedef struct {
    motion_metrics_t metrics;
    SemaphoreHandle_t lock;
} mm_task_ctx_t;

/**
 * @brief Initialize the storage context and synchronization
 * 
 * @param ctx Context structure to initialize
 * @param cfg Configuration parameters
 * @return ESP_OK on success
 */
esp_err_t mm_task_init(mm_task_ctx_t *ctx, const motion_monitor_config_t *cfg);

/**
 * @brief Update metrics and synchronize with the system registry
 * 
 * @param ctx Context structure
 * @param metrics New metrics received from remote node
 * @return ESP_OK on success
 */
esp_err_t mm_task_update_metrics(mm_task_ctx_t *ctx, const motion_metrics_t *metrics);
