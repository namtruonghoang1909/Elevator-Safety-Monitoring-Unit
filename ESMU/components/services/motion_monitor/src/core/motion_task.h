/**
 * @file motion_task.h
 * @brief FreeRTOS task and synchronization for motion monitor
 */

#pragma once

#include "motion_monitor.h"
#include "motion_processor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

/**
 * @brief Motion monitor task context
 */
typedef struct {
    motion_metrics_t metrics;
    motion_processor_t processor;
    SemaphoreHandle_t lock;
    TaskHandle_t task_handle;
    uint8_t mpu_id;
    bool is_calibrated;
} mm_task_ctx_t;

/**
 * @brief Initialize the task context and synchronization
 * 
 * @param ctx Context structure to initialize
 * @param cfg Configuration parameters
 * @return ESP_OK on success
 */
esp_err_t mm_task_init(mm_task_ctx_t *ctx, const motion_monitor_config_t *cfg);

/**
 * @brief Start the background polling task
 * 
 * @param ctx Context structure
 * @param cfg Configuration parameters
 * @return ESP_OK on success
 */
esp_err_t mm_task_start(mm_task_ctx_t *ctx, const motion_monitor_config_t *cfg);

/**
 * @brief Central processing entry point for a single sample (External/Internal)
 * 
 * @param ctx Context structure
 * @param raw Raw sensor sample
 */
void mm_task_process_sample(mm_task_ctx_t *ctx, const mpu6050_scaled_data_t *raw);
