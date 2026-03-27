/**
 * @file motion_kinematics.h
 * @brief Kinematics module for ESMU Motion Monitor
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "protocol_types.h"

typedef struct {
    float lin_accel_z;      /**< Linear Z-acceleration in Gs (Gravity removed) */
    float velocity;         /**< Current estimated velocity (m/s) */
    float displacement;     /**< Total displacement (m) */
    motion_state_t state;   /**< Current motion state */
} kinematics_data_t;

/**
 * @brief Process kinematics data based on new acceleration sample
 */
void motion_kinematics_process(kinematics_data_t *data, int16_t raw_accel_z, int16_t offset_z, float dt);

/**
 * @brief Reset velocity and displacement to zero (ZUPT)
 */
void motion_kinematics_reset_integrals(kinematics_data_t *data);
