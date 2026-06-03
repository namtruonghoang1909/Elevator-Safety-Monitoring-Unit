/**
 * @file i2c_platform.h
 * @brief Thread-safe I2C Platform Abstraction for STM32
 * 
 * Symmetrical with Gateway ESP32 I2C Platform for easy driver porting.
 */

#pragma once

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief I2C Platform status codes
 */
typedef enum {
    PLATFORM_I2C_OK = 0,
    PLATFORM_I2C_ERROR,
    PLATFORM_I2C_BUSY,
    PLATFORM_I2C_TIMEOUT
} platform_i2c_status_t;

/**
 * @brief Initialize the I2C Platform (Creates mutexes)
 */
platform_i2c_status_t platform_i2c_init(void);

// ─────────────────────────────────────────────
// Register-level Operations
// ─────────────────────────────────────────────

/**
 * @brief Write a single byte to a register
 */
platform_i2c_status_t platform_i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t val);

/**
 * @brief Read a single byte from a register
 */
platform_i2c_status_t platform_i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *val);

/**
 * @brief Read multiple consecutive registers (burst read)
 */
platform_i2c_status_t platform_i2c_read_consecutive_regs(uint8_t dev_addr, uint8_t start_reg, uint8_t *data_buf, uint16_t len);

// ─────────────────────────────────────────────
// Low-level Byte / Bytes Operations 
// ─────────────────────────────────────────────

/**
 * @brief Write a single raw byte (no register address)
 */
platform_i2c_status_t platform_i2c_write_byte(uint8_t dev_addr, uint8_t value);

/**
 * @brief Write multiple raw bytes in one transaction
 */
platform_i2c_status_t platform_i2c_write_bytes(uint8_t dev_addr, const uint8_t *data, uint16_t len);

/**
 * @brief Read a single raw byte
 */
platform_i2c_status_t platform_i2c_read_byte(uint8_t dev_addr, uint8_t *out_val);

/**
 * @brief Check if a device is ready
 */
bool platform_i2c_is_ready(uint8_t dev_addr);
