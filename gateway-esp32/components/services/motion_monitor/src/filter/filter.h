/**
 * @file filter.h
 * @brief EMA filtering and magnitude calculations for MPU6050 data
 */

#pragma once

#include "mpu6050.h"

/**
 * @brief Apply Exponential Moving Average (EMA) filter to a raw sample
 * 
 * Formula: Output[n] = alpha * Input[n] + (1 - alpha) * Output[n-1]
 * 
 * @param raw Input raw scaled data from driver
 * @param filtered [in/out] Current filtered state structure
 * @param alpha Smoothing factor (0.0 - 1.0, lower is smoother)
 */
void mm_filter_apply(const mpu6050_scaled_data_t *raw, mpu6050_scaled_data_t *filtered, float alpha);

/**
 * @brief Calculate derived metrics (Linear Z and Shake Magnitude)
 * 
 * Isolates movement from gravity and computes horizontal vibration magnitude.
 * 
 * @param filtered Smoothed sensor data
 * @param gravity_z The calibrated 1.0g baseline for the Z-axis
 * @param lin_z_out [out] Resulting vertical linear acceleration (g)
 * @param shake_mag_out [out] Resulting horizontal vibration magnitude (g)
 */
void mm_filter_calculate_metrics(const mpu6050_scaled_data_t *filtered, float gravity_z, float *lin_z_out, float *shake_mag_out);
