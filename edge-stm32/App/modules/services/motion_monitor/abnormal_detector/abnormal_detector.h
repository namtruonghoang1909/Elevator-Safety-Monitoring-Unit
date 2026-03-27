/**
 * @file abnormal_detector.h
 * @brief Fault and Emergency detection module for ESMU
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "protocol_types.h"
#include "mpu6050.h"

typedef struct {
    health_status_t current_health;
    fault_code_t current_fault;
    int16_t raw_fault_val;
    uint8_t debounce_cnt;
} detector_result_t;

void abnormal_detector_process(detector_result_t *res, mpu6050_raw_data_t raw, int16_t offset_z, int16_t *offsets_gyro);
void abnormal_detector_reset(detector_result_t *res);
