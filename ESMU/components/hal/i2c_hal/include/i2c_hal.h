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
 * @brief Initialize an I2C bus
 *
 * @param bus_id       Bus ID (0 or 1 for ESP32)
 * @param sda_pin      GPIO for SDA
 * @param scl_pin      GPIO for SCL
 * @param clk_speed_hz Clock speed in Hz (e.g. 100000, 400000)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_hal_init(uint8_t bus_id, gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t clk_speed_hz);

/**
 * @brief Add an I2C device to a specific bus
 *
 * @param dev_id       Unique device ID (0 to MAX_DEVICES_NUM-1)
 * @param dev_addr     7-bit I2C address
 * @param dev_name     Device name (string, stored as pointer – must remain valid)
 * @param scl_speed_hz SCL frequency for this device (0 = use bus default)
 * @param on_bus_id    Which bus to attach to (0 or 1)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_hal_add_device(uint8_t dev_id, uint8_t dev_addr, const char *dev_name,
                             uint32_t scl_speed_hz, uint8_t on_bus_id);

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