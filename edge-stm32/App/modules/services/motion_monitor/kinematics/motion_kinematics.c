/**
 * @file motion_kinematics.c
 * @brief Kinematic integration and state detection
 */

#include "motion_kinematics.h"
#include "motion_filters.h"
#include <math.h>
#include <stdlib.h>

#define NOISE_FLOOR_G        0.015f
#define ACCEL_LSB_PER_G      4096.0f
#define GRAVITY_MSS          9.80665f

void motion_kinematics_process(kinematics_data_t *data, int16_t raw_accel_z, int16_t offset_z, float dt) {
    int16_t raw_delta = raw_accel_z - offset_z;
    if (abs(raw_delta) < 300) raw_delta = 0; // Increased to 300 LSB noise gate

    float raw_lin_g = (float)raw_delta / ACCEL_LSB_PER_G;
    data->lin_accel_z = raw_lin_g;

    float accel_mss = data->lin_accel_z * GRAVITY_MSS;
    
    if (data->lin_accel_z == 0.0f) {
        data->velocity *= 0.95f; 
        if (fabsf(data->velocity) < 0.001f) data->velocity = 0.0f;
    } else {
        data->velocity += accel_mss * dt;
    }

    data->displacement += data->velocity * dt;

    if (data->lin_accel_z > 0.05f) {
        data->state = MOTION_STATE_MOVING_UP;
    } else if (data->lin_accel_z < -0.05f) {
        data->state = MOTION_STATE_MOVING_DOWN;
    } else if (fabsf(data->velocity) < 0.01f) {
        data->state = MOTION_STATE_STATIONARY;
    }
}

void motion_kinematics_reset_integrals(kinematics_data_t *data) {
    data->velocity = 0.0f;
    data->displacement = 0.0f;
}
