/**
 * @file motion_monitor.c
 * @brief Professional Motion & Safety Monitor for ESMU Edge Node
 */

#include "motion_monitor.h"
#include "system_registry.h"
#include "edge_logger.h"
#include "edge_telemetry.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include <stdlib.h>
#include "stm32f1xx_hal.h"

// ─────────────────────────────────────────────
// Raw Integer Thresholds (LSB) 
// Based on: 4096 LSB/g and 32.8 LSB/deg/s
// ─────────────────────────────────────────────
#define RAW_THRESHOLD_FREE_FALL  1433    /**< 0.35g * 4096 */
#define RAW_THRESHOLD_IMPACT     1228    /**< 0.30g * 4096 */
#define RAW_THRESHOLD_VIB_HIGH   492     /**< 15.0 deg/s * 32.8 (Stricter) */
#define RAW_THRESHOLD_VIB_MED    229     /**< 7.0 deg/s * 32.8 (Stricter) */

#define DEBOUNCE_FAULT_CYCLES    10      /**< 100ms (10 * 10ms) */
#define EMERGENCY_COOLDOWN_MS    3000    /**< 3 second interval between broadcasts */

// ─────────────────────────────────────────────
// Private variables
// ─────────────────────────────────────────────
static motion_metrics_t current_metrics = {0};
static motion_monitor_config_t current_cfg = { .filter_alpha = 0.25f };

// Calibration Offsets (Raw bits)
static int16_t raw_offset_x = 0, raw_offset_y = 0, raw_offset_z = 4096;
static int16_t raw_offset_gx = 0, raw_offset_gy = 0, raw_offset_gz = 0;

// Filtered states (Physical units for reporting)
static float filt_z = 1.0f;
static float filt_vib = 0.0f;

// Fault Debouncing & Diagnostic tracking
static uint8_t emergency_debounce_cnt = 0;
static uint32_t last_emergency_broadcast_tick = 0;
static health_status_t worst_health_in_window = HEALTH_STABLE;
static fault_code_t worst_fault_code = FAULT_NONE;
static int16_t worst_raw_val = 0;

static TaskHandle_t motionTaskHandle = NULL;

// ─────────────────────────────────────────────
// Core Logic
// ─────────────────────────────────────────────

static void motion_monitor_process(void) {
    mpu6050_raw_data_t raw;
    if (!mpu6050_read_raw(&raw)) return;

    // 1. Instantaneous Health Check (RAW Integer Detection)
    // Fast integer arithmetic for the 10ms loop
    int16_t adj_gx = raw.gyro_x - raw_offset_gx;
    int16_t adj_gy = raw.gyro_y - raw_offset_gy;
    int16_t adj_gz = raw.gyro_z - raw_offset_gz;
    
    // Sum of absolutes is a fast approximation for vibration magnitude
    int32_t raw_vib_sum = abs(adj_gx) + abs(adj_gy) + abs(adj_gz);
    
    // Total acceleration magnitude
    int32_t raw_accel_mag = (int32_t)sqrtf((float)raw.accel_x*raw.accel_x + (float)raw.accel_y*raw.accel_y + (float)raw.accel_z*raw.accel_z);

    health_status_t instant_health = HEALTH_STABLE;
    fault_code_t instant_fault = FAULT_NONE;
    int16_t instant_raw_val = 0;

    // Detection Logic
    if (raw_accel_mag < RAW_THRESHOLD_FREE_FALL) {
        instant_health = HEALTH_EMERGENCY;
        instant_fault = FAULT_FREEFALL;
        instant_raw_val = (int16_t)raw_accel_mag;
    } 
    else if (abs(raw.accel_z - raw_offset_z) > RAW_THRESHOLD_IMPACT) {
        instant_health = HEALTH_EMERGENCY;
        instant_fault = FAULT_EMERGENCY_STOP;
        instant_raw_val = (int16_t)(raw.accel_z - raw_offset_z);
    }
    else if (raw_vib_sum > (RAW_THRESHOLD_VIB_HIGH * 2)) {
        instant_health = HEALTH_EMERGENCY;
        instant_fault = FAULT_SHAKE;
        instant_raw_val = (int16_t)(raw_vib_sum / 2);
    }
    else if (raw_vib_sum > (RAW_THRESHOLD_VIB_MED * 2)) {
        instant_health = HEALTH_WARNING;
        instant_fault = FAULT_SHAKE;
        instant_raw_val = (int16_t)(raw_vib_sum / 2);
    }

    // 2. State Debounce Integrator
    if (instant_health != HEALTH_STABLE) {
        if (emergency_debounce_cnt < DEBOUNCE_FAULT_CYCLES) {
            emergency_debounce_cnt++;
        } else {
            // Sustained Fault (100ms)
            if (current_metrics.health_status != instant_health) {
                if (instant_health == HEALTH_EMERGENCY) {
                    // Apply Cooldown: Only broadcast if 3 seconds have passed
                    uint32_t now = xTaskGetTickCount();
                    if (now - last_emergency_broadcast_tick >= pdMS_TO_TICKS(EMERGENCY_COOLDOWN_MS)) {
                        float phys_val = (instant_fault == FAULT_SHAKE) ? 
                                         ((float)instant_raw_val / 32.8f) : ((float)instant_raw_val / 4096.0f);
                        edge_telemetry_broadcast_emergency(instant_fault, 5, current_metrics.state, (int16_t)(phys_val * 100), (uint16_t)xTaskGetTickCount());
                        last_emergency_broadcast_tick = now;
                    }
                }
                current_metrics.health_status = instant_health;
            }
        }
        
        // Track for OLED diagnostic (Requires 30ms persistence to filter I2C noise)
        if (emergency_debounce_cnt >= 3) {
            if (instant_health >= worst_health_in_window) {
                worst_health_in_window = instant_health;
                worst_fault_code = instant_fault;
                if (abs(instant_raw_val) > abs(worst_raw_val)) worst_raw_val = instant_raw_val;
            }
        }
    } else {
        if (emergency_debounce_cnt > 0) {
            emergency_debounce_cnt--;
        } else {
            // Sustained Stability
            current_metrics.health_status = HEALTH_STABLE;
        }
    }

    // 3. Scaling & Smoothing (For Metrics and Logging)
    float acc_z = (float)raw.accel_z / 4096.0f;
    filt_z = (acc_z * current_cfg.filter_alpha) + (filt_z * (1.0f - current_cfg.filter_alpha));

    float cur_vib_phys = sqrtf(((float)adj_gx*adj_gx + (float)adj_gy*adj_gy + (float)adj_gz*adj_gz)) / 32.8f;
    filt_vib = (cur_vib_phys * 0.15f) + (filt_vib * 0.85f);

    float lin_z_phys = filt_z - ((float)raw_offset_z / 4096.0f);

    // Update Registry Data
    current_metrics.vibration = filt_vib;
    current_metrics.speed = 0; // No encoder available yet, placeholder
    
    // Calculate health score based on vibration (0-15 deg/s mapped to 100-0)
    float score = 100.0f - (filt_vib * (100.0f / 15.0f));
    if (score < 0) score = 0;
    current_metrics.health_score = (uint8_t)score;

    if (current_metrics.health_status == HEALTH_STABLE) {
        if (lin_z_phys > 0.05f) current_metrics.state = MOTION_STATE_MOVING_UP;
        else if (lin_z_phys < -0.05f) current_metrics.state = MOTION_STATE_MOVING_DOWN;
        else current_metrics.state = MOTION_STATE_STATIONARY;
    } else if (current_metrics.health_status == HEALTH_EMERGENCY) {
        current_metrics.state = (instant_fault == FAULT_FREEFALL) ? MOTION_STATE_FREE_FALL : MOTION_STATE_SHAKING;
    }

    system_registry_t reg;
    if (system_registry_read(&reg)) {
        reg.metrics = current_metrics;
        reg.raw = (mpu6050_scaled_data_t){.accel_z_g = acc_z}; // Partial fill
        system_registry_write(&reg);
    }

    // 4. OLED Diagnostic Logger (300ms)
    static uint32_t last_log = 0;
    if (xTaskGetTickCount() - last_log > pdMS_TO_TICKS(300)) {
        last_log = xTaskGetTickCount();

        if (worst_health_in_window != HEALTH_STABLE) {
            float scaled_v = (worst_fault_code == FAULT_SHAKE) ? 
                             ((float)worst_raw_val / 32.8f) : ((float)worst_raw_val / 40.96f);
            edge_logger_printf("F:%d V:%d", (int)worst_fault_code, (int)scaled_v);
        } else {
            edge_logger_printf("Z:%d V:%d S:%d", (int)(lin_z_phys*100), (int)filt_vib, current_metrics.health_score);
        }
        
        worst_health_in_window = HEALTH_STABLE;
        worst_raw_val = 0;
    }
}

static void motion_task(void *argument) {
    TickType_t last_wake = xTaskGetTickCount();
    for(;;) {
        motion_monitor_process();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
    }
}

// ─────────────────────────────────────────────
// Global Functions
// ─────────────────────────────────────────────

bool motion_monitor_init(const motion_monitor_config_t *cfg) {
    if (cfg) current_cfg = *cfg;
    if (!mpu6050_init()) return false;
    
    edge_logger_print("CALIBRATING...");
    motion_monitor_calibrate();
    edge_logger_print("CALIBRATED...");

    return true;
}

bool motion_monitor_start(void) {
    if (motionTaskHandle != NULL) return true;
    BaseType_t ret = xTaskCreate(motion_task, "MotionTask", 512, NULL, tskIDLE_PRIORITY + 3, &motionTaskHandle);
    return (ret == pdPASS);
}

void motion_monitor_calibrate(void) {
    mpu6050_raw_data_t sample;
    int32_t sx = 0, sy = 0, sz = 0;
    int32_t gx = 0, gy = 0, gz = 0;

    for (int i = 0; i < 100; i++) {
        if (mpu6050_read_raw(&sample)) {
            sx += sample.accel_x; sy += sample.accel_y; sz += sample.accel_z;
            gx += sample.gyro_x; gy += sample.gyro_y; gz += sample.gyro_z;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    raw_offset_x = (int16_t)(sx / 100);
    raw_offset_y = (int16_t)(sy / 100);
    raw_offset_z = (int16_t)(sz / 100);
    raw_offset_gx = (int16_t)(gx / 100);
    raw_offset_gy = (int16_t)(gy / 100);
    raw_offset_gz = (int16_t)(gz / 100);
}

void motion_monitor_get_metrics(motion_metrics_t *metrics) {
    if (metrics) *metrics = current_metrics;
}
