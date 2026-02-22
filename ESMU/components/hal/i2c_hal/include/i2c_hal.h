#pragma once

#include <stdint.h>
#include <stddef.h>
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define MAX_I2C_BUS_NUM    2
#define MAX_DEVICES_NUM    5   
#define I2C_TIMEOUT_MS     50

/**
 * @brief I2C bus information
 */
typedef struct {
    i2c_master_bus_handle_t bus_handle;
    uint8_t                 id;
} i2c_bus_info_t;

/**
 * @brief I2C device information
 */
typedef struct i2c_device_info_t {
    i2c_master_dev_handle_t dev_handle;
    uint8_t                 id;
    uint8_t                 address;
    const char             *name;   // pointer to device name string
} i2c_device_info_t;

/**
 * @brief Initialize an I2C bus and assign/return its ID
 *
 * @param bus_id_out   [in/out] Pointer to bus ID variable.
 *                     - If *bus_id_out == 255 (or any invalid value), auto-assign first free bus
 *                     - If *bus_id_out is valid (0 or 1), use that bus ID (if available)
 * @param sda_pin      GPIO for SDA
 * @param scl_pin      GPIO for SCL
 * @param clk_speed_hz Desired clock speed (0 = default)
 * @return ESP_OK on success
 *         ESP_ERR_INVALID_ARG / ESP_ERR_NOT_FOUND / etc. on failure
 */
esp_err_t i2c_hal_init(uint8_t *bus_id_out, gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t clk_speed_hz);


/**
 * @brief Add a device to a bus and assign/return its internal device ID
 *
 * @param dev_id_out   [out] Assigned device ID (written by function)
 * @param dev_addr     7-bit slave address
 * @param dev_name     Optional name (pointer stored – must remain valid)
 * @param scl_speed_hz Device-specific SCL speed (0 = use bus default)
 * @param on_bus_id    Target bus ID
 * @return ESP_OK on success
 *         ESP_ERR_* on failure
 */
esp_err_t i2c_hal_add_device(uint8_t *dev_id_out,
                             uint8_t dev_addr,
                             const char *dev_name,
                             uint32_t scl_speed_hz,
                             uint8_t on_bus_id);

/**
 * @brief Write a single byte to a register on a device
 *
 * @param dev_id Device ID
 * @param reg    Register address
 * @param val    Value to write
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_hal_write_reg(uint8_t dev_id, uint8_t reg, uint8_t val);

/**
 * @brief Read a single byte from a register on a device
 *
 * @param dev_id Device ID
 * @param reg    Register address
 * @param val    [out] Read value
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_hal_read_reg(uint8_t dev_id, uint8_t reg, uint8_t *val);

/**
 * @brief Reading values on a auto increasing address
 *
 * @param dev_id  Device's ID
 * @param start_reg register address to start reading
 * @param data_buf [out] Read values
 * @param buf_len length of data_buf
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_hal_read_consecutive_regs(uint8_t dev_id,
                                        const uint8_t start_reg,
                                        uint8_t *data_buf, size_t buf_len);

/**
 * @brief Deinitialize a specific I2C bus
 * 
 * @param I2C bus's ID
 */
esp_err_t i2c_hal_deinit_bus(uint8_t bus_id);

/**
 * @brief Remove a device (free resources)
 * 
 * @param I2C device's ID
 */
esp_err_t i2c_hal_remove_device(uint8_t dev_id);