/**
 * @file mpu6050.h
 * @brief MPU-6050 6-axis motion tracking device driver interface
 *
 * Provides:
 *   - Initialization with configurable ranges, sample rate, DLPF
 *   - Raw and scaled data reading (accel in g, gyro in °/s, temp in °C)
 *   - Register verification and auto-recovery on misconfiguration
 *   - Helper functions for debugging and magnitude calculation
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "../mpu6050_reg.h"  // register map & constants

// ─────────────────────────────────────────────
// Data structures
// ─────────────────────────────────────────────

/**
 * @brief Raw 16-bit sensor readings (directly from registers)
 */
typedef struct {
    int16_t accel_x;     // Accelerometer X
    int16_t accel_y;     // Y
    int16_t accel_z;     // Z
    int16_t temperature; // Temperature (raw)
    int16_t gyro_x;      // Gyroscope X
    int16_t gyro_y;      // Y
    int16_t gyro_z;      // Z
} mpu6050_raw_data_t;

/**
 * @brief Scaled physical values
 */
typedef struct {
    float accel_x_g;     // Accelerometer X (g)
    float accel_y_g;     // Y (g)
    float accel_z_g;     // Z (g)
    float gyro_x_dps;    // Gyroscope X (°/s)
    float gyro_y_dps;    // Y (°/s)
    float gyro_z_dps;    // Z (°/s)
    float temperature_c; // Temperature (°C)
} mpu6050_scaled_data_t;

/**
 * @brief Configuration structure for mpu6050_init()
 */
typedef struct {
    uint8_t  address;               // I2C slave address (usually 0x68)
    const char *name;               // Optional name for logging
    uint32_t scl_speed_hz;          // SCL clock speed (Hz)
    uint8_t  bus_id;                // Bus ID from i2c_bus_init()
    uint8_t  accel_full_scale;      // MPU6050_ACCEL_FS_xxx
    uint8_t  gyro_full_scale;       // MPU6050_GYRO_FS_xxx
    uint8_t  sample_rate_div;       // Sample rate divider (0–255)
    bool     use_gyro_pll;          // Use X gyro PLL clock (recommended)
    bool     enable_digital_filter; // Enable DLPF (44 Hz bandwidth)
} mpu6050_config_t;

// ─────────────────────────────────────────────
// Initialization, Reset , Existance
// ─────────────────────────────────────────────

/**
 * @brief Initialize MPU6050 (wake up, defaults, apply config, verify presence)
 *
 * @param cfg         Configuration (address, ranges, sample rate, etc.)
 * @param dev_id_out  [out] Assigned device ID
 * @return ESP_OK on success
 */
esp_err_t mpu6050_init(const mpu6050_config_t *cfg, uint8_t *dev_id_out);

/**
 * @brief Perform soft reset of the MPU6050
 *
 * @param dev_id Device ID
 * @return ESP_OK on success
 */
esp_err_t mpu6050_reset(uint8_t dev_id);

/**
 * @brief Check if MPU6050 is present by reading WHO_AM_I register
 *
 * @param dev_id Device ID
 * @return true if WHO_AM_I == 0x68
 */
bool mpu6050_is_present(uint8_t dev_id);

// ─────────────────────────────────────────────
// Data Reading
// ─────────────────────────────────────────────

/**
 * @brief Read raw 14-byte burst (accel + temperature + gyro)
 *
 * @param dev_id Device ID
 * @param data   [out] Raw data structure
 * @return ESP_OK on success
 */
esp_err_t mpu6050_read_raw(uint8_t dev_id, mpu6050_raw_data_t *data);

/**
 * @brief Read scaled physical values (g, °/s, °C)
 *
 * @param dev_id Device ID
 * @param data   [out] Scaled data structure
 * @return ESP_OK on success
 */
esp_err_t mpu6050_read_scaled(uint8_t dev_id, mpu6050_scaled_data_t *data);

/**
 * @brief Get magnitude of acceleration vector (useful for motion/free-fall detection)
 *
 * @param data Scaled data structure
 * @return Magnitude in g
 */
float mpu6050_get_accel_magnitude(const mpu6050_scaled_data_t *data);
