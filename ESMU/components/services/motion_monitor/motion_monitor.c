#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "motion_monitor.h"
#include "motion_filter.h"
#include "motion_fsm.h"

static const char *TAG = "MOTION_MONITOR";

#define CALIBRATION_SAMPLES     200

typedef struct {
    motion_metrics_t metrics;
    mpu6050_scaled_data_t filtered_raw;
    float gravity_offset_z;
    float alpha;
    uint8_t mpu_id;
    SemaphoreHandle_t lock;
    TaskHandle_t task_handle;
    uint32_t fsm_timer_ms;
    motion_state_t fsm_pending;
    bool is_calibrated;
} motion_context_t;

static motion_context_t s_ctx;

/**
 * @brief Central processing entry point (Used by task and injection)
 */
static void _process_sample(const mpu6050_scaled_data_t *raw) {
    xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
    
    // 1. Math: Filter and Magnitude
    mm_filter_apply(raw, &s_ctx.filtered_raw, s_ctx.alpha);
    mm_filter_calculate_metrics(&s_ctx.filtered_raw, s_ctx.gravity_offset_z, 
                                &s_ctx.metrics.lin_accel_z, &s_ctx.metrics.shake_mag);
    
    // 2. Logic: State Machine
    s_ctx.metrics.state = mm_fsm_update(s_ctx.metrics.state, s_ctx.metrics.lin_accel_z, 
                                        &s_ctx.fsm_timer_ms, &s_ctx.fsm_pending);
    
    s_ctx.metrics.gyro_roll_dps = s_ctx.filtered_raw.gyro_x_dps;
    s_ctx.metrics.gyro_pitch_dps = s_ctx.filtered_raw.gyro_y_dps;
    s_ctx.metrics.last_update = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    xSemaphoreGive(s_ctx.lock);
}

static void motion_monitor_task(void *arg) {
    TickType_t last_wake_time = xTaskGetTickCount();
    mpu6050_scaled_data_t raw;

    while (1) {
        if (mpu6050_read_scaled(s_ctx.mpu_id, &raw) == ESP_OK) {
            _process_sample(&raw);
        }
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10));
    }
}

esp_err_t motion_monitor_init(const motion_monitor_config_t *cfg) {
    if (!cfg) return ESP_ERR_INVALID_ARG;

    memset(&s_ctx, 0, sizeof(motion_context_t));
    s_ctx.mpu_id = cfg->mpu_dev_id;
    s_ctx.alpha = (cfg->filter_alpha > 0) ? cfg->filter_alpha : 0.2f;
    s_ctx.gravity_offset_z = 1.0f;
    s_ctx.metrics.state = MOTION_STATE_STATIONARY;
    
    s_ctx.lock = xSemaphoreCreateMutex();
    if (!s_ctx.lock) return ESP_ERR_NO_MEM;

    if (s_ctx.mpu_id != 0xFF) {
        uint32_t stack = (cfg->task_stack > 0) ? cfg->task_stack : 4096;
        uint32_t prio = (cfg->task_priority > 0) ? cfg->task_priority : configMAX_PRIORITIES - 2;
        xTaskCreate(motion_monitor_task, "motion_task", stack, NULL, prio, &s_ctx.task_handle);
    }

    ESP_LOGI(TAG, "Service initialized (Alpha: %.2f)", s_ctx.alpha);
    return ESP_OK;
}

esp_err_t motion_monitor_get_metrics(motion_metrics_t *out) {
    if (!out || !s_ctx.lock) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
    memcpy(out, &s_ctx.metrics, sizeof(motion_metrics_t));
    xSemaphoreGive(s_ctx.lock);
    return ESP_OK;
}

motion_state_t motion_monitor_get_state(void) {
    motion_state_t state;
    xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
    state = s_ctx.metrics.state;
    xSemaphoreGive(s_ctx.lock);
    return state;
}

esp_err_t motion_monitor_calibrate(void) {
    if (s_ctx.mpu_id == 0xFF) {
        s_ctx.gravity_offset_z = 1.0f;
        s_ctx.is_calibrated = true;
        return ESP_OK;
    }
    
    float sum_z = 0;
    mpu6050_scaled_data_t raw;
    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        if (mpu6050_read_scaled(s_ctx.mpu_id, &raw) == ESP_OK) {
            sum_z += raw.accel_z_g;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
    s_ctx.gravity_offset_z = sum_z / CALIBRATION_SAMPLES;
    s_ctx.is_calibrated = true;
    s_ctx.filtered_raw.accel_z_g = s_ctx.gravity_offset_z;
    xSemaphoreGive(s_ctx.lock);

    ESP_LOGI(TAG, "Calibration complete (Baseline: %.4f g)", s_ctx.gravity_offset_z);
    return ESP_OK;
}

esp_err_t motion_monitor_set_filter_alpha(float alpha) {
    if (alpha <= 0.0f || alpha > 1.0f) return ESP_ERR_INVALID_ARG;
    s_ctx.alpha = alpha;
    return ESP_OK;
}

esp_err_t motion_monitor_inject_sample(const mpu6050_scaled_data_t *raw_sample) {
    if (!raw_sample) return ESP_ERR_INVALID_ARG;
    _process_sample(raw_sample);
    return ESP_OK;
}
