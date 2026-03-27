/**
 * @file abnormal_detector.c
 * @brief Implementation of fault detection logic
 */

#include "abnormal_detector.h"
#include <math.h>
#include <stdlib.h>

#define RAW_THRESHOLD_FREE_FALL  1433
#define RAW_THRESHOLD_IMPACT     1228
#define RAW_THRESHOLD_VIB_HIGH   100     /**< 10.6 deg/s * 32.8 */ // newly updated by dev
#define RAW_THRESHOLD_VIB_MED    50     /**< 4.5 deg/s * 32.8 */ // newly updated by dev
#define DEBOUNCE_CYCLES          10      /**< Increased to 100ms for stability */

void abnormal_detector_process(detector_result_t *res, mpu6050_raw_data_t raw, int16_t offset_z, int16_t *offsets_gyro) {
    int32_t raw_accel_mag = (int32_t)sqrtf((float)raw.accel_x*raw.accel_x + (float)raw.accel_y*raw.accel_y + (float)raw.accel_z*raw.accel_z);
    
    int32_t adj_gx = raw.gyro_x - offsets_gyro[0];
    int32_t adj_gy = raw.gyro_y - offsets_gyro[1];
    int32_t adj_gz = raw.gyro_z - offsets_gyro[2];
    int32_t raw_vib_sum = abs(adj_gx) + abs(adj_gy) + abs(adj_gz);

    health_status_t instant_health = HEALTH_STABLE;
    fault_code_t instant_fault = FAULT_NONE;
    int16_t val = 0;

    bool is_already_fault = (res->current_health != HEALTH_STABLE);
    int32_t high_thresh = RAW_THRESHOLD_VIB_HIGH * 2;
    int32_t med_thresh = RAW_THRESHOLD_VIB_MED * 2;
    
    if (is_already_fault) {
        high_thresh = (high_thresh * 7) / 10;
        med_thresh = (med_thresh * 7) / 10;
    }

    if (raw_accel_mag < RAW_THRESHOLD_FREE_FALL) {
        instant_health = HEALTH_EMERGENCY;
        instant_fault = FAULT_FREEFALL;
        val = (int16_t)raw_accel_mag;
    } else if (abs(raw.accel_z - offset_z) > RAW_THRESHOLD_IMPACT) {
        instant_health = HEALTH_EMERGENCY;
        instant_fault = FAULT_EMERGENCY_STOP;
        val = (int16_t)(raw.accel_z - offset_z);
    } else if (raw_vib_sum > high_thresh) {
        instant_health = HEALTH_EMERGENCY;
        instant_fault = FAULT_SHAKE;
        val = (int16_t)(raw_vib_sum / 2);
    } else if (raw_vib_sum > med_thresh) {
        instant_health = HEALTH_WARNING;
        instant_fault = FAULT_SHAKE;
        val = (int16_t)(raw_vib_sum / 2);
    }
// 2. Debouncing Logic
if (instant_health != HEALTH_STABLE) {
    // Increment debounce when fault is active
    if (res->debounce_cnt < DEBOUNCE_CYCLES) {
        res->debounce_cnt++;
    }

    // Once debounced, lock the fault
    if (res->debounce_cnt >= DEBOUNCE_CYCLES) {
        res->current_health = instant_health;
        res->current_fault = instant_fault;
        res->raw_fault_val = val;
    }
} else {
    // Fast-recovery: decrement debounce when signal is stable
    if (res->debounce_cnt > 0) {
        res->debounce_cnt--;
    }

    // Clear fault immediately when debounce reaches 0
    if (res->debounce_cnt == 0) {
        res->current_health = HEALTH_STABLE;
        res->current_fault = FAULT_NONE;
        res->raw_fault_val = 0;
    }
}
}

void abnormal_detector_reset(detector_result_t *res) {
    res->current_health = HEALTH_STABLE;
    res->current_fault = FAULT_NONE;
    res->raw_fault_val = 0;
    res->debounce_cnt = 0;
}
