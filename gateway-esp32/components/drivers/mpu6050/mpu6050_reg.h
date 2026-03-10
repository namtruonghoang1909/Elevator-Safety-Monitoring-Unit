/**
 * @file mpu6050_reg.h
 * @brief Register map and constants for MPU-6050
 */

#pragma once

// ─────────────────────────────────────────────
// Key register addresses
// ─────────────────────────────────────────────

#define MPU6050_REG_SMPLRT_DIV      0x19
#define MPU6050_REG_CONFIG          0x1A
#define MPU6050_REG_GYRO_CONFIG     0x1B
#define MPU6050_REG_ACCEL_CONFIG    0x1C
#define MPU6050_REG_FIFO_EN         0x23
#define MPU6050_REG_INT_PIN_CFG     0x37
#define MPU6050_REG_INT_ENABLE      0x38
#define MPU6050_REG_INT_STATUS      0x3A
#define MPU6050_REG_ACCEL_XOUT_H    0x3B   // start of 14-byte burst: accel(6) + temp(2) + gyro(6)
#define MPU6050_REG_TEMP_OUT_H      0x41
#define MPU6050_REG_GYRO_XOUT_H     0x43
#define MPU6050_REG_USER_CTRL       0x6A
#define MPU6050_REG_PWR_MGMT_1      0x6B
#define MPU6050_REG_PWR_MGMT_2      0x6C
#define MPU6050_REG_WHO_AM_I        0x75

// ─────────────────────────────────────────────
// Full-scale range configuration values
// ─────────────────────────────────────────────

#define MPU6050_ACCEL_FS_2G         (0u << 3)   // ±2 g
#define MPU6050_ACCEL_FS_4G         (1u << 3)   // ±4 g
#define MPU6050_ACCEL_FS_8G         (2u << 3)   // ±8 g
#define MPU6050_ACCEL_FS_16G        (3u << 3)   // ±16 g

#define MPU6050_GYRO_FS_250_DPS     (0u << 3)   // ±250 °/s
#define MPU6050_GYRO_FS_500_DPS     (1u << 3)   // ±500 °/s
#define MPU6050_GYRO_FS_1000_DPS    (2u << 3)   // ±1000 °/s
#define MPU6050_GYRO_FS_2000_DPS    (3u << 3)   // ±2000 °/s

// ─────────────────────────────────────────────
// Sensitivity constants (LSB per unit)
// ─────────────────────────────────────────────

#define MPU6050_ACCEL_SENS_2G       16384.0f
#define MPU6050_ACCEL_SENS_4G       8192.0f
#define MPU6050_ACCEL_SENS_8G       4096.0f
#define MPU6050_ACCEL_SENS_16G      2048.0f

#define MPU6050_GYRO_SENS_250       131.0f
#define MPU6050_GYRO_SENS_500       65.5f
#define MPU6050_GYRO_SENS_1000      32.8f
#define MPU6050_GYRO_SENS_2000      16.4f

// Temperature scaling constants
#define MPU6050_TEMP_SENS           340.0f
#define MPU6050_TEMP_OFFSET         36.53f

// Default I2C address (AD0 = GND)
#define MPU6050_DEFAULT_I2C_ADDR    0x68

// Expected WHO_AM_I register value
#define MPU6050_WHO_AM_I_VALUE      0x70 // usually be 0x68 but the mpu6050 returned 0x70 so we adapt to it