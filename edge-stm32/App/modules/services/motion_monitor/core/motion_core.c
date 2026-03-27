/**
 * @file motion_core.c
 * @brief Implementation of the motion monitor background task
 */

#include "motion_core.h"
#include "mpu6050.h"
#include "system_registry.h"
#include "edge_logger.h"
#include "edge_telemetry.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define STATUS_LOG_INTERVAL_MS   500
#define EDGE_LOG_FAULT_INTERVAL_MS 500

static TaskHandle_t motionTaskHandle = NULL;
static motion_calibration_t calib = {0};
static kinematics_data_t current_kinematics = {0};
static detector_result_t current_detectors = {0};

static uint32_t last_status_log_tick = 0;
static uint32_t last_edge_fault_log_tick = 0;
static bool was_active_last_cycle = false;

static const char* get_fault_text(fault_code_t fault) {
    switch (fault) {
        case FAULT_SHAKE: return "Shake";
        case FAULT_FREEFALL: return "FF";
        case FAULT_EMERGENCY_STOP: return "HStop";
        default: return "Unknown";
    }
}

static void motion_task(void *argument) {
    TickType_t last_wake = xTaskGetTickCount();
    mpu6050_raw_data_t raw;
    
    for(;;) {
        uint32_t now = xTaskGetTickCount();
        system_registry_t reg;
        if (!system_registry_read(&reg)) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }

        if (!reg.is_monitoring_active) {
            if (was_active_last_cycle) { edge_logger_print("MONITOR STOPPED"); was_active_last_cycle = false; }
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        if (!was_active_last_cycle) {
            was_active_last_cycle = true;
            
            // RESET watchdog baseline to prevent immediate STALE fault
            // We set it to current time to give the task a fresh window
            reg.last_update_tick = xTaskGetTickCount();
            system_registry_write(&reg);
        }

        if (mpu6050_read_raw(&raw)) {
            // Update freshness tick consistently
            reg.last_update_tick = xTaskGetTickCount();
            abnormal_detector_process(&current_detectors, raw, calib.accel_offsets[2], calib.gyro_offsets);
            motion_kinematics_process(&current_kinematics, raw.accel_z, calib.accel_offsets[2], 0.01f);

            // Fault reporting logic
            if (current_detectors.current_health == HEALTH_EMERGENCY && reg.metrics.vibration > 5.0f) {
                // ALWAYS call telemetry - it handles its own 5s cooldown internaly
                edge_telemetry_broadcast_emergency(
                    current_detectors.current_fault, 
                    5, 
                    (uint8_t)reg.state, 
                    current_detectors.raw_fault_val, 
                    (uint16_t)(now / configTICK_RATE_HZ)
                );

                // Local OLED logging with 500ms cooldown to keep display readable
                if (now - last_edge_fault_log_tick >= pdMS_TO_TICKS(EDGE_LOG_FAULT_INTERVAL_MS)) {
                    edge_logger_printf("F:%s RAW:%d", get_fault_text(current_detectors.current_fault), (int)current_detectors.raw_fault_val);                    
                    last_edge_fault_log_tick = now;
                }
            } 

            reg.metrics.state = current_kinematics.state;
            // Calculate adjusted values using deadband to ensure stability
            // 400 LSB deadband on each raw gyro channel
            int16_t dx = raw.gyro_x - calib.gyro_offsets[0];
            int16_t dy = raw.gyro_y - calib.gyro_offsets[1];
            int16_t dz = raw.gyro_z - calib.gyro_offsets[2];

            if (abs(dx) < 400) dx = 0;
            if (abs(dy) < 400) dy = 0;
            if (abs(dz) < 400) dz = 0;

            reg.metrics.vibration = (float)(abs(dx) + abs(dy) + abs(dz)) / 32.8f;

            float score = 100.0f - (reg.metrics.vibration * 6.6f);
            reg.metrics.health_score = (score < 0) ? 0 : (uint8_t)score;
            system_registry_write(&reg);
            if (now - last_status_log_tick >= pdMS_TO_TICKS(STATUS_LOG_INTERVAL_MS)) {
                edge_logger_printf("Z:%d V:%d S:%d", (int)(current_kinematics.lin_accel_z * 100), (int)reg.metrics.vibration, reg.metrics.health_score);
                // Log raw sensor values for troubleshooting
                last_status_log_tick = now;
            }
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
    }
}

bool motion_core_init(void) { if (!mpu6050_init()) return false;  return true; }
bool motion_core_start(void) { if (motionTaskHandle != NULL) return true; BaseType_t ret = xTaskCreate(motion_task, "MotionCore", 1024, NULL, tskIDLE_PRIORITY + 3, &motionTaskHandle); return (ret == pdPASS); }
void motion_core_calibrate(void) {
    int32_t sums[6] = {0}; mpu6050_raw_data_t sample; const int num_samples = 500;
    for (int i = 0; i < num_samples; i++) {
        if (mpu6050_read_raw(&sample)) { sums[0] += sample.accel_x; sums[1] += sample.accel_y; sums[2] += sample.accel_z; sums[3] += sample.gyro_x; sums[4] += sample.gyro_y; sums[5] += sample.gyro_z; }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    calib.accel_offsets[0] = (int16_t)(sums[0] / num_samples); calib.accel_offsets[1] = (int16_t)(sums[1] / num_samples); calib.accel_offsets[2] = (int16_t)(sums[2] / num_samples);
    calib.gyro_offsets[0] = (int16_t)(sums[3] / num_samples); calib.gyro_offsets[1] = (int16_t)(sums[4] / num_samples); calib.gyro_offsets[2] = (int16_t)(sums[5] / num_samples);
}
void motion_core_get_results(kinematics_data_t *kin, detector_result_t *det) { if (kin) *kin = current_kinematics; if (det) *det = current_detectors; }
