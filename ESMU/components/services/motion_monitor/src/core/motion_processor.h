/**
 * @file motion_processor.h
 * @brief Logic orchestrator for motion, balance, and filtering
 */

#pragma once

#include "motion_monitor.h"
#include "mpu6050.h"

/**
 * @brief Processor state containing all internal persistent data
 */
typedef struct {
    mpu6050_scaled_data_t filtered_raw;
    float offset_x;
    float offset_y;
    float gravity_offset_z;
    float alpha;
    uint32_t fsm_timer_ms;
    motion_state_t fsm_pending;
} motion_processor_t;

/**
 * @brief Initialize the processor logic
 * 
 * @param proc Processor state structure
 * @param alpha Initial EMA filter alpha
 */
void mm_processor_init(motion_processor_t *proc, float alpha);

/**
 * @brief Process a single raw sensor sample
 * 
 * @param proc Processor state
 * @param raw Input raw scaled sample
 * @param metrics [in/out] Metrics structure to update
 */
void mm_processor_run(motion_processor_t *proc, const mpu6050_scaled_data_t *raw, motion_metrics_t *metrics);

/**
 * @brief Update the sensor baseline (used during calibration)
 * 
 * @param proc Processor state
 * @param x Baseline X
 * @param y Baseline Y
 * @param z Baseline Z (Gravity)
 */
void mm_processor_set_baseline(motion_processor_t *proc, float x, float y, float z);
