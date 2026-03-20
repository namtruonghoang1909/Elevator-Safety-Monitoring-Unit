/**
 * @file mpu6050.h
 * @brief MPU6050 6-Axis IMU Driver for STM32 (I2C)
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

// ─────────────────────────────────────────────
// MPU6050 Registers
// ─────────────────────────────────────────────
#define MPU_ADDR            0x68
#define MPU_WHO_AM_I_VAL    0x70
#define MPU_WHO_AM_I        0x75
#define MPU_SMPLRT_DIV      0x19
#define MPU_CONFIG          0x1A
#define MPU_GYRO_CONFIG     0x1B
#define MPU_ACCEL_CONFIG    0x1C
#define MPU_ACCEL_XOUT_H    0x3B
#define MPU_TEMP_OUT_H      0x41
#define MPU_GYRO_XOUT_H     0x43
#define MPU_PWR_MGMT_1      0x6B
#define MPU_PWR_MGMT_2      0x6C

// ─────────────────────────────────────────────
// Types
// ─────────────────────────────────────────────

typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} mpu6050_raw_data_t;

typedef struct {
    float accel_x_g;
    float accel_y_g;
    float accel_z_g;
    float temp_c;
    float gyro_x_ds;
    float gyro_y_ds;
    float gyro_z_ds;
} mpu6050_scaled_data_t;

/**
 * @brief Initialize MPU6050
 * 
 * Sets: 
 * - Clock: Gyro X
 * - Accel: +/- 8g
 * - Gyro: +/- 1000 deg/s
 * - Sample Rate: 100Hz (1kHz / (1+9))
 */
bool mpu6050_init(void);

/**
 * @brief Read all raw 14 bytes (Accel, Temp, Gyro)
 */
bool mpu6050_read_raw(mpu6050_raw_data_t *raw);

/**
 * @brief Get scaled data in g and deg/s
 */
bool mpu6050_get_scaled(mpu6050_scaled_data_t *scaled);

/**
 * @brief Test connection via WHO_AM_I
 */
bool mpu6050_test_connection(void);
