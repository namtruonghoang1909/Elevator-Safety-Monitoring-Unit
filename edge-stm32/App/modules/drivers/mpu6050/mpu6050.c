/**
 * @file mpu6050.c
 * @brief MPU6050 6-Axis IMU Driver implementation for STM32
 */

#include "mpu6050.h"
#include "bsp_i2c.h"
#include <string.h>

// ─────────────────────────────────────────────
// Scaling Factors (Based on Register Config)
// ─────────────────────────────────────────────
// +/- 8g = 4096 LSB/g (Bit 3-4 = 2 in Reg 28)
#define ACCEL_SCALE     4096.0f
// +/- 1000 deg/s = 32.8 LSB/deg/s (Bit 3-4 = 2 in Reg 27)
#define GYRO_SCALE      32.8f

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

bool mpu6050_test_connection(void) {
    uint8_t who_am_i = 0;
    if (bsp_i2c_read_reg(MPU_ADDR, MPU_WHO_AM_I, &who_am_i) != BSP_I2C_OK) {
        return false;
    }
    return (who_am_i == MPU_WHO_AM_I_VAL);
}

bool mpu6050_init(void) {
    // 1. Wake up (Power Mgmt 1) - Clear Sleep, Set Clock to Gyro X
    if (bsp_i2c_write_reg(MPU_ADDR, MPU_PWR_MGMT_1, 0x01) != BSP_I2C_OK) return false;

    // 2. Sample Rate Divider = 9 (100Hz if Gyro Out is 1kHz)
    if (bsp_i2c_write_reg(MPU_ADDR, MPU_SMPLRT_DIV, 0x09) != BSP_I2C_OK) return false;

    // 3. Config (DLPF = 3, ~44Hz bandwidth)
    if (bsp_i2c_write_reg(MPU_ADDR, MPU_CONFIG, 0x03) != BSP_I2C_OK) return false;

    // 4. Gyro Config (+/- 1000 deg/s)
    if (bsp_i2c_write_reg(MPU_ADDR, MPU_GYRO_CONFIG, 0x10) != BSP_I2C_OK) return false;

    // 5. Accel Config (+/- 8g)
    if (bsp_i2c_write_reg(MPU_ADDR, MPU_ACCEL_CONFIG, 0x10) != BSP_I2C_OK) return false;

    return mpu6050_test_connection();
}

bool mpu6050_read_raw(mpu6050_raw_data_t *raw) {
    uint8_t buffer[14];
    
    // Burst read 14 bytes starting from ACCEL_XOUT_H
    if (bsp_i2c_read_consecutive_regs(MPU_ADDR, MPU_ACCEL_XOUT_H, buffer, 14) != BSP_I2C_OK) {
        return false;
    }

    // Combine High/Low bytes (Big Endian)
    raw->accel_x = (int16_t)((buffer[0] << 8) | buffer[1]);
    raw->accel_y = (int16_t)((buffer[2] << 8) | buffer[3]);
    raw->accel_z = (int16_t)((buffer[4] << 8) | buffer[5]);
    raw->temp    = (int16_t)((buffer[6] << 8) | buffer[7]);
    raw->gyro_x  = (int16_t)((buffer[8] << 8) | buffer[9]);
    raw->gyro_y  = (int16_t)((buffer[10] << 8) | buffer[11]);
    raw->gyro_z  = (int16_t)((buffer[12] << 8) | buffer[13]);

    return true;
}

bool mpu6050_get_scaled(mpu6050_scaled_data_t *scaled) {
    mpu6050_raw_data_t raw;
    if (!mpu6050_read_raw(&raw)) return false;

    scaled->accel_x_g = (float)raw.accel_x / ACCEL_SCALE;
    scaled->accel_y_g = (float)raw.accel_y / ACCEL_SCALE;
    scaled->accel_z_g = (float)raw.accel_z / ACCEL_SCALE;

    // Temp = Reg / 340 + 36.53
    scaled->temp_c = ((float)raw.temp / 340.0f) + 36.53f;

    scaled->gyro_x_ds = (float)raw.gyro_x / GYRO_SCALE;
    scaled->gyro_y_ds = (float)raw.gyro_y / GYRO_SCALE;
    scaled->gyro_z_ds = (float)raw.gyro_z / GYRO_SCALE;

    return true;
}
