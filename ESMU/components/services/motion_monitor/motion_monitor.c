#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "motion_monitor.h"
#include "core/motion_task.h"

static const char *TAG = "MOTION_MONITOR";
#define CALIBRATION_SAMPLES 200

static mm_task_ctx_t s_ctx;

esp_err_t motion_monitor_init(const motion_monitor_config_t *cfg) {
    esp_err_t err = mm_task_init(&s_ctx, cfg);
    if (err != ESP_OK) return err;

    err = mm_task_start(&s_ctx, cfg);
    if (err != ESP_OK) return err;

    ESP_LOGI(TAG, "Service initialized (Alpha: %.2f)", s_ctx.processor.alpha);
    return ESP_OK;
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

esp_err_t motion_monitor_calibrate(void) {
    if (s_ctx.mpu_id == 0xFF) {
        mm_processor_set_baseline(&s_ctx.processor, 0.0f, 0.0f, 1.0f);
        s_ctx.is_calibrated = true;
        return ESP_OK;
    }
    
    float sum_x = 0, sum_y = 0, sum_z = 0;
    mpu6050_scaled_data_t raw;
    int valid_samples = 0;

    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        if (mpu6050_read_scaled(s_ctx.mpu_id, &raw) == ESP_OK) {
            sum_x += raw.accel_x_g;
            sum_y += raw.accel_y_g;
            sum_z += raw.accel_z_g;
            valid_samples++;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (valid_samples < (CALIBRATION_SAMPLES / 2)) {
        ESP_LOGE(TAG, "Calibration failed: Too many I2C errors");
        return ESP_FAIL;
    }

    xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
    mm_processor_set_baseline(&s_ctx.processor, 
                              sum_x / valid_samples, 
                              sum_y / valid_samples, 
                              sum_z / valid_samples);
    s_ctx.is_calibrated = true;
    xSemaphoreGive(s_ctx.lock);

    ESP_LOGI(TAG, "Calibration complete (Offsets X:%.3f, Y:%.3f, Z:%.3f)", 
             s_ctx.processor.offset_x, s_ctx.processor.offset_y, s_ctx.processor.gravity_offset_z);
    return ESP_OK;
}

esp_err_t motion_monitor_set_filter_alpha(float alpha) {
    if (alpha <= 0.0f || alpha > 1.0f) return ESP_ERR_INVALID_ARG;
    if (!s_ctx.lock) return ESP_FAIL;
    
    xSemaphoreTake(s_ctx.lock, portMAX_DELAY);
    s_ctx.processor.alpha = alpha;
    xSemaphoreGive(s_ctx.lock);
    return ESP_OK;
}

esp_err_t motion_monitor_inject_sample(const mpu6050_scaled_data_t *raw_sample) {
    mm_task_process_sample(&s_ctx, raw_sample);
    return ESP_OK;
}
