/**
 * @file i2c_platform.h
 * @brief Modern I2C master bus & device management abstraction (Platform Layer)
 *
 * Built on ESP-IDF i2c_master.h (no legacy driver).
 * Supports both register-style access and low-level byte/bytes operations.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define MAX_I2C_BUS_NUM    2
#define MAX_DEVICES_NUM    10
#define I2C_TIMEOUT_MS     500

/**
 * @brief I2C bus information structure
 */
typedef struct {
    i2c_master_bus_handle_t bus_handle;
    uint8_t                 id;
} i2c_bus_info_t;

/**
 * @brief I2C device information structure
 */
typedef struct {
    i2c_master_dev_handle_t dev_handle;
    uint8_t                 id;
    uint8_t                 address;
    const char             *name;   // Optional name (pointer stored, not copied)
} i2c_device_info_t;

// ─────────────────────────────────────────────
// Bus & Device Management
// ─────────────────────────────────────────────

/**
 * @brief Initialize an I2C master bus and assign/return its ID
 */
esp_err_t platform_i2c_bus_init(uint8_t *bus_id_out, gpio_num_t sda_pin, gpio_num_t scl_pin);

/**
 * @brief Scan the I2C bus for connected devices
 */
esp_err_t platform_i2c_scan(uint8_t bus_id, uint8_t *addr_list, size_t max_results, size_t *found_count);

/**
 * @brief Add a device to a bus and assign/return its internal device ID
 */
esp_err_t platform_i2c_add_device(uint8_t *dev_id_out,
                             uint8_t dev_addr,
                             const char *dev_name,
                             uint32_t scl_speed_hz,
                             uint8_t on_bus_id);

/**
 * @brief Deinitialize (delete) a specific I2C bus
 */
esp_err_t platform_i2c_deinit_bus(uint8_t bus_id);

/**
 * @brief Remove a device from its bus (free resources)
 */
esp_err_t platform_i2c_remove_device(uint8_t dev_id);

// ─────────────────────────────────────────────
// Register-level Operations
// ─────────────────────────────────────────────

/**
 * @brief Write a single byte to a register on a device
 */
esp_err_t platform_i2c_write_reg(uint8_t dev_id, uint8_t reg, uint8_t val);

/**
 * @brief Read a single byte from a register on a device
 */
esp_err_t platform_i2c_read_reg(uint8_t dev_id, uint8_t reg, uint8_t *val);

/**
 * @brief Read multiple consecutive registers (burst read)
 */
esp_err_t platform_i2c_read_consecutive_regs(uint8_t dev_id,
                                        uint8_t start_reg,
                                        uint8_t *data_buf,
                                        size_t buf_len);

// ─────────────────────────────────────────────
// Low-level Byte / Bytes Operations 
// ─────────────────────────────────────────────

/**
 * @brief Write a single raw byte (no register address)
 *        Useful for control bytes, raw commands, etc.
 */
esp_err_t platform_i2c_write_byte(uint8_t dev_id, uint8_t value);

/**
 * @brief Write multiple raw bytes in one transaction
 *        Useful for command sequences or pixel data blocks
 */
esp_err_t platform_i2c_write_bytes(uint8_t dev_id, const uint8_t *data, size_t len);

/**
 * @brief Read a single raw byte (no register address sent first)
 *        Useful for reading status or after previous addressing
 */
esp_err_t platform_i2c_read_byte(uint8_t dev_id, uint8_t *out_val);

/**
 * @brief Read multiple bytes starting from a register (burst read)
 */
esp_err_t platform_i2c_read_bytes(uint8_t dev_id,
                             uint8_t start_reg,
                             uint8_t *buf,
                             size_t len);
