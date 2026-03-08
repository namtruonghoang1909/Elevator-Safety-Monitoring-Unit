#include <math.h>
#include "motion_filter.h"

static inline float _ema(float input, float prev, float alpha) {
    return (alpha * input) + (1.0f - alpha) * prev;
}

void mm_filter_apply(const mpu6050_scaled_data_t *raw, mpu6050_scaled_data_t *filtered, float alpha) {
    filtered->accel_x_g = _ema(raw->accel_x_g, filtered->accel_x_g, alpha);
    filtered->accel_y_g = _ema(raw->accel_y_g, filtered->accel_y_g, alpha);
    filtered->accel_z_g = _ema(raw->accel_z_g, filtered->accel_z_g, alpha);
    filtered->gyro_x_dps = _ema(raw->gyro_x_dps, filtered->gyro_x_dps, alpha);
    filtered->gyro_y_dps = _ema(raw->gyro_y_dps, filtered->gyro_y_dps, alpha);
    filtered->gyro_z_dps = _ema(raw->gyro_z_dps, filtered->gyro_z_dps, alpha);
}

void mm_filter_calculate_metrics(const mpu6050_scaled_data_t *filtered, float gravity_z, float *lin_z_out, float *shake_mag_out) {
    if (lin_z_out) {
        *lin_z_out = filtered->accel_z_g - gravity_z;
    }
    if (shake_mag_out) {
        *shake_mag_out = sqrtf(filtered->accel_x_g * filtered->accel_x_g + filtered->accel_y_g * filtered->accel_y_g);
    }
}
