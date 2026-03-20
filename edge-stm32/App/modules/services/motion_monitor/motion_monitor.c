/**
 * @file motion_monitor.c
 * @brief Professional Motion & Safety Monitor for ESMU Edge Node
 * 
 * Logic Focus:
 * 1. Safety Faults: Free Fall and Sudden Impact (High-G stops).
 * 2. Maintenance: Vibration monitoring (Gyro-based) for ride quality.
 * 3. Motion: Directional tracking (Up/Down/Still).
 */

#include "motion_monitor.h"
#include "system_registry.h"
#include "edge_logger.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include "stm32f1xx_hal.h"

// ─────────────────────────────────────────────
// Physical Thresholds (Professional Tuning)
// ─────────────────────────────────────────────
#define THRESHOLD_FREE_FALL_G   0.35f   /**< Total magnitude near zero-G */
#define THRESHOLD_IMPACT_G      1.60f   /**< Sudden stop or hit (Absolute Z force) */
#define THRESHOLD_VIB_HIGH_DS   50.0f   /**< High vibration (Maintenance needed) */
#define THRESHOLD_VIB_MED_DS    20.0f   /**< Moderate vibration */
#define THRESHOLD_TRAVEL_G      0.08f   /**< Vertical acceleration for movement */

// ─────────────────────────────────────────────
// Private variables
// ─────────────────────────────────────────────
static motion_metrics_t current_metrics = {0};
static motion_monitor_config_t current_cfg = { .filter_alpha = 0.25f };

// Calibration Offsets
static float offset_x = 0.0f, offset_y = 0.0f, offset_z = 1.0f;

// Filtered states
static float filt_x = 0.0f, filt_y = 0.0f, filt_z = 1.0f;

static TaskHandle_t motionTaskHandle = NULL;

// ─────────────────────────────────────────────
// Core Logic
// ─────────────────────────────────────────────

static void motion_monitor_process(void) {
    mpu6050_scaled_data_t sample;
    if (!mpu6050_get_scaled(&sample)) return;

    // 1. EMA Noise Filtering
    filt_x = (sample.accel_x_g * current_cfg.filter_alpha) + (filt_x * (1.0f - current_cfg.filter_alpha));
    filt_y = (sample.accel_y_g * current_cfg.filter_alpha) + (filt_y * (1.0f - current_cfg.filter_alpha));
    filt_z = (sample.accel_z_g * current_cfg.filter_alpha) + (filt_z * (1.0f - current_cfg.filter_alpha));

    // 2. Physics Calculations
    float lin_z = filt_z - offset_z;  // Vertical force relative to gravity
    float vib_mag = sqrtf(sample.gyro_x_ds * sample.gyro_x_ds + 
                          sample.gyro_y_ds * sample.gyro_y_ds + 
                          sample.gyro_z_ds * sample.gyro_z_ds);
    
    float total_accel = sqrtf(filt_x*filt_x + filt_y*filt_y + filt_z*filt_z);

    // 3. Fault Detection (Highest Priority)
    if (total_accel < THRESHOLD_FREE_FALL_G) {
        current_metrics.state = MOTION_STATE_FREE_FALL;
    } 
    else if (fabsf(lin_z) > (THRESHOLD_IMPACT_G - 1.0f)) {
        current_metrics.state = MOTION_STATE_SHAKING; // Map to Shaking/Impact for now
    }
    // 4. Motion & Vibration Status
    else {
        if (lin_z > THRESHOLD_TRAVEL_G) current_metrics.state = MOTION_STATE_MOVING_UP;
        else if (lin_z < -THRESHOLD_TRAVEL_G) current_metrics.state = MOTION_STATE_MOVING_DOWN;
        else current_metrics.state = MOTION_STATE_STATIONARY;
    }

    current_metrics.avg_tilt = vib_mag; // Re-purposing field for Vibration magnitude
    if (vib_mag > current_metrics.max_tilt) current_metrics.max_tilt = vib_mag;

    // 5. Update System Registry (Whiteboard)
    system_registry_t reg;
    if (system_registry_read(&reg)) {
        reg.metrics = current_metrics;
        reg.raw = sample;
        system_registry_write(&reg);
    }

    // 6. OLED Maintenance Logger (Every 500ms)
    static uint32_t last_log = 0;
    if (xTaskGetTickCount() - last_log > pdMS_TO_TICKS(500)) {
        last_log = xTaskGetTickCount();
        
        // Line A: Safety Health
        const char* health = "HEALTH:OK";
        if (current_metrics.state == MOTION_STATE_FREE_FALL) health = "!! FALL !!";
        else if (fabsf(lin_z) > 0.5f) health = "!! IMPACT !!";

        // Line B: Ride Quality (Vibration)
        const char* vib_status = "VIB:LOW";
        if (vib_mag > THRESHOLD_VIB_HIGH_DS) vib_status = "VIB:CRIT";
        else if (vib_mag > THRESHOLD_VIB_MED_DS) vib_status = "VIB:WARN";

        // Line C: Travel Direction
        const char* travel = "STILL";
        if (current_metrics.state == MOTION_STATE_MOVING_UP) travel = "UP";
        else if (current_metrics.state == MOTION_STATE_MOVING_DOWN) travel = "DOWN";

        edge_logger_printf("%s", health);
        edge_logger_printf("%s | %s", vib_status, travel);
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
    edge_logger_print("SYSTEM READY");
    return true;
}

void motion_monitor_start(void) {
    xTaskCreate(motion_task, "MotionTask", 1024, NULL, tskIDLE_PRIORITY + 3, &motionTaskHandle);
}

void motion_monitor_calibrate(void) {
    mpu6050_scaled_data_t sample;
    float sx = 0, sy = 0, sz = 0;
    for (int i = 0; i < 100; i++) {
        if (mpu6050_get_scaled(&sample)) {
            sx += sample.accel_x_g; sy += sample.accel_y_g; sz += sample.accel_z_g;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    offset_x = sx / 100.0f; offset_y = sy / 100.0f; offset_z = sz / 100.0f;
    filt_x = offset_x; filt_y = offset_y; filt_z = offset_z;
}

void motion_monitor_get_metrics(motion_metrics_t *metrics) {
    if (metrics) *metrics = current_metrics;
}
