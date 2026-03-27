/**
 * @file motion_core.h
 * @brief Core task and synchronization for Motion Monitor
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "motion_filters.h"
#include "motion_kinematics.h"
#include "abnormal_detector.h"

typedef struct {
    int16_t accel_offsets[3];
    int16_t gyro_offsets[3];
} motion_calibration_t;

bool motion_core_init(void);
bool motion_core_start(void);
void motion_core_calibrate(void);
void motion_core_get_results(kinematics_data_t *kin, detector_result_t *det);
