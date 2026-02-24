#pragma once

#include <stdint.h>
#include <stddef.h>
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define MAX_I2C_BUS_NUM    2
#define MAX_DEVICES_NUM    8   // Slightly increased for flexibility
#define I2C_TIMEOUT_MS     100

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

/**
 * @brief Initialize an I2C master bus and assign/return its ID
 *
 * @param bus_id_out   [in/out] Pointer to bus ID variable.
 *                     - If *bus_id_out == 255 (or any invalid value), auto-assign first free bus
 *                     - If *bus_id_out is valid (0 or 1), use that bus ID (if available)
 * @param sda_pin      GPIO for SDA
 * @param scl_pin      GPIO for SCL
 * @return ESP_OK on success
 *         ESP_ERR_* on failure
 */
esp_err_t i2c_bus_init(uint8_t *bus_id_out, gpio_num_t sda_pin, gpio_num_t scl_pin);

/**
 * @brief Scan the I2C bus for connected devices
 *
 * Scans addresses from 0x08 to 0x77 (standard 7-bit range excluding reserved addresses)
 * and returns a list of addresses that respond (ACK).
 *
 * @param bus_id      The bus ID returned from i2c_bus_init()
 * @param addr_list   [out] Buffer to store detected addresses
 * @param max_results Maximum number of addresses that can be stored in addr_list
 * @param found_count [out] Number of devices actually found
 * @return ESP_OK on success (even if no devices found)
 *         ESP_ERR_* on failure
 */
esp_err_t i2c_scan(uint8_t bus_id, uint8_t *addr_list, size_t max_results, size_t *found_count);

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
esp_err_t i2c_add_device(uint8_t *dev_id_out,
                         uint8_t dev_addr,
                         const char *dev_name,
                         uint32_t scl_speed_hz,
                         uint8_t on_bus_id);

/**
 * @brief Write a single byte to a register on a device
 *
 * @param dev_id Device ID from i2c_add_device
 * @param reg    Register address
 * @param val    Value to write
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_write_reg(uint8_t dev_id, uint8_t reg, uint8_t val);

/**
 * @brief Read a single byte from a register on a device
 *
 * @param dev_id Device ID
 * @param reg    Register address
 * @param val    [out] Read value
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_read_reg(uint8_t dev_id, uint8_t reg, uint8_t *val);

/**
 * @brief Read multiple consecutive registers (burst read)
 *
 * @param dev_id     Device ID
 * @param start_reg  Starting register address
 * @param data_buf   [out] Buffer to store read data
 * @param buf_len    Number of bytes to read
 * @return ESP_OK on success
 */
esp_err_t i2c_read_consecutive_regs(uint8_t dev_id,
                                    uint8_t start_reg,
                                    uint8_t *data_buf, size_t buf_len);

/**
 * @brief Deinitialize (delete) a specific I2C bus
 *
 * @param bus_id Bus ID to deinitialize
 * @return ESP_OK on success
 */
esp_err_t i2c_deinit_bus(uint8_t bus_id);

/**
 * @brief Remove a device from its bus (free resources)
 *
 * @param dev_id Device ID to remove
 * @return ESP_OK on success
 */
esp_err_t i2c_remove_device(uint8_t dev_id);