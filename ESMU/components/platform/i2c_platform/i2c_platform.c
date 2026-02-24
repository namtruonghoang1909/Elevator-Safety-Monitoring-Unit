#include "i2c_platform.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"   // for pdMS_TO_TICKS

#define INVALID_ID_MARKER   255

// Changed TAG to "i2c_bus" for clarity (was "i2c_hal")
static const char *TAG = "i2c_bus";

static i2c_bus_info_t    bus[MAX_I2C_BUS_NUM]    = {{0}};
static i2c_device_info_t device[MAX_DEVICES_NUM] = {{0}};

/**
 * @brief Initialize an I2C master bus and assign/return its ID
 *        (Changed function name from i2c_hal_init to i2c_bus_init for clarity)
 */
esp_err_t i2c_bus_init(uint8_t *bus_id_out, gpio_num_t sda_pin, gpio_num_t scl_pin)
{
    if (bus_id_out == NULL) {
        ESP_LOGE(TAG, "bus_id_out pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t requested = *bus_id_out;
    uint8_t assigned  = INVALID_ID_MARKER;

    // Auto-assign if caller passed invalid/marker value
    if (requested >= MAX_I2C_BUS_NUM) {
        for (uint8_t i = 0; i < MAX_I2C_BUS_NUM; i++) {
            if (bus[i].bus_handle == NULL) {
                assigned = i;
                break;
            }
        }
        if (assigned == INVALID_ID_MARKER) {
            ESP_LOGE(TAG, "No free I2C bus slot available (max %d)", MAX_I2C_BUS_NUM);
            return ESP_ERR_NOT_FOUND;
        }
    } else {
        assigned = requested;
        if (assigned >= MAX_I2C_BUS_NUM) {
            ESP_LOGE(TAG, "Requested bus_id %d is out of range", requested);
            return ESP_ERR_INVALID_ARG;
        }
    }

    // Already initialized? → reuse (idempotent)  // Improved warning message
    if (bus[assigned].bus_handle != NULL) {
        ESP_LOGW(TAG, "I2C bus %d already initialized → reusing", assigned);
        *bus_id_out = assigned;
        return ESP_OK;
    }

    bus[assigned].id = assigned;

    i2c_master_bus_config_t bus_cfg = {
        .clk_source        = I2C_CLK_SRC_DEFAULT,
        .i2c_port          = (i2c_port_t)assigned,
        .sda_io_num        = sda_pin,
        .scl_io_num        = scl_pin,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
        .intr_priority     = 0,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_cfg, &bus[assigned].bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C bus %d: %s", assigned, esp_err_to_name(ret));
        return ret;
    }

    // Improved log with pin numbers
    ESP_LOGI(TAG, "I2C bus %d initialized: SDA=%d, SCL=%d",
             assigned, sda_pin, scl_pin);

    *bus_id_out = assigned;
    return ESP_OK;
}

/**
 * @brief Scan the I2C bus for connected slave devices using probe
 *        (Added temporary log level suppression for NACK spam - common improvement)
 */
esp_err_t i2c_scan(uint8_t bus_id, uint8_t *addr_list, size_t max_results, size_t *found_count)
{
    if (bus_id >= MAX_I2C_BUS_NUM || bus[bus_id].bus_handle == NULL) {
        ESP_LOGE(TAG, "Invalid or uninitialized bus ID: %d", bus_id);
        return ESP_ERR_INVALID_STATE;
    }

    if (addr_list == NULL || found_count == NULL || max_results == 0) {
        ESP_LOGE(TAG, "Invalid parameters for i2c_scan");
        return ESP_ERR_INVALID_ARG;
    }

    *found_count = 0;
    ESP_LOGI(TAG, "Scanning I2C bus %d (0x08–0x77)...", bus_id);

    // Temporarily suppress NACK spam from i2c.master (restored at end)
    esp_log_level_t old_level = esp_log_get_default_level();
    esp_log_level_set("i2c.master", ESP_LOG_ERROR);

    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        esp_err_t ret = i2c_master_probe(bus[bus_id].bus_handle,
                                         addr,
                                         pdMS_TO_TICKS(I2C_TIMEOUT_MS));

        if (ret == ESP_OK) {
            if (*found_count < max_results) {
                addr_list[*found_count] = addr;
                (*found_count)++;
            }
            ESP_LOGI(TAG, "Device found at 0x%02X", addr);
        }
    }

    esp_log_level_set("i2c.master", old_level);

    if (*found_count == 0) {
        ESP_LOGW(TAG, "No I2C devices detected on bus %d", bus_id);
    } else {
        ESP_LOGI(TAG, "Scan complete. Found %zu device(s)", *found_count);
    }

    return ESP_OK;
}

/**
 * @brief Add a slave device to a bus and assign a unique device ID
 *        (Improved error messages and partial cleanup on failure)
 */
esp_err_t i2c_add_device(uint8_t *dev_id_out,
                         uint8_t dev_addr,
                         const char *dev_name,
                         uint32_t scl_speed_hz,
                         uint8_t on_bus_id)
{
    if (dev_id_out == NULL) {
        ESP_LOGE(TAG, "dev_id_out pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    *dev_id_out = INVALID_ID_MARKER;

    if (on_bus_id >= MAX_I2C_BUS_NUM || bus[on_bus_id].bus_handle == NULL) {
        ESP_LOGE(TAG, "Bus %d not initialized or invalid", on_bus_id);
        return ESP_ERR_INVALID_STATE;
    }

    // Find free slot
    uint8_t assigned = INVALID_ID_MARKER;
    for (uint8_t i = 0; i < MAX_DEVICES_NUM; i++) {
        if (device[i].dev_handle == NULL) {
            assigned = i;
            break;
        }
    }

    if (assigned == INVALID_ID_MARKER) {
        ESP_LOGE(TAG, "No free device slot (max %d)", MAX_DEVICES_NUM);
        return ESP_ERR_NO_MEM;
    }

    device[assigned].id       = assigned;
    device[assigned].address  = dev_addr;
    device[assigned].name     = dev_name;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = dev_addr,
        .scl_speed_hz    = (scl_speed_hz > 0) ? scl_speed_hz : 400000,
        .scl_wait_us     = 0,
    };

    esp_err_t ret = i2c_master_bus_add_device(bus[on_bus_id].bus_handle,
                                              &dev_cfg,
                                              &device[assigned].dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device @ 0x%02X on bus %d: %s",
                 dev_addr, on_bus_id, esp_err_to_name(ret));
        // Clean partial state on failure
        device[assigned].dev_handle = NULL;
        device[assigned].address    = 0;
        device[assigned].name       = NULL;
        return ret;
    }

    ESP_LOGI(TAG, "Added device %d: %s @ 0x%02X on bus %d",
             assigned, dev_name ? dev_name : "?", dev_addr, on_bus_id);

    *dev_id_out = assigned;
    return ESP_OK;
}

/**
 * @brief Write a single byte to a register on a device
 *        (Added register & value to error log for better debugging)
 */
esp_err_t i2c_write_reg(uint8_t dev_id, uint8_t reg, uint8_t val)
{
    if (dev_id >= MAX_DEVICES_NUM || device[dev_id].dev_handle == NULL) {
        ESP_LOGE(TAG, "Device %d not initialized", dev_id);
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t buf[2] = {reg, val};
    esp_err_t ret = i2c_master_transmit(device[dev_id].dev_handle,
                                        buf, 2,
                                        pdMS_TO_TICKS(I2C_TIMEOUT_MS));

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Write reg 0x%02X = 0x%02X failed on dev %d: %s",
                 reg, val, dev_id, esp_err_to_name(ret));
    }

    return ret;
}

/**
 * @brief Read a single byte from a register on a device
 *        (Added register to error log)
 */
esp_err_t i2c_read_reg(uint8_t dev_id, uint8_t reg, uint8_t *val)
{
    if (dev_id >= MAX_DEVICES_NUM || device[dev_id].dev_handle == NULL || val == NULL) {
        ESP_LOGE(TAG, "Invalid params for read on dev %d", dev_id);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = i2c_master_transmit_receive(device[dev_id].dev_handle,
                                                &reg, 1,
                                                val, 1,
                                                pdMS_TO_TICKS(I2C_TIMEOUT_MS));

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read reg 0x%02X failed on dev %d: %s",
                 reg, dev_id, esp_err_to_name(ret));
    }

    return ret;
}

/**
 * @brief Read multiple consecutive registers (burst read)
 *        (Improved error log with start reg & length)
 */
esp_err_t i2c_read_consecutive_regs(uint8_t dev_id,
                                    const uint8_t start_reg,
                                    uint8_t *data_buf, size_t buf_len)
{
    if (dev_id >= MAX_DEVICES_NUM || device[dev_id].dev_handle == NULL) {
        ESP_LOGE(TAG, "Device %d not initialized", dev_id);
        return ESP_ERR_INVALID_STATE;
    }
    if (data_buf == NULL || buf_len == 0) {
        ESP_LOGE(TAG, "Invalid buffer or length for burst read on dev %d", dev_id);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = i2c_master_transmit_receive(device[dev_id].dev_handle,
                                                &start_reg, 1,
                                                data_buf, buf_len,
                                                pdMS_TO_TICKS(I2C_TIMEOUT_MS));

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Burst read from 0x%02X (len %u) failed on dev %d: %s",
                 start_reg, (unsigned)buf_len, dev_id, esp_err_to_name(ret));
    }

    return ret;
}

/**
 * @brief Deinitialize (delete) a specific I2C bus
 */
esp_err_t i2c_deinit_bus(uint8_t bus_id)
{
    if (bus_id >= MAX_I2C_BUS_NUM || bus[bus_id].bus_handle == NULL) {
        return ESP_OK;
    }

    esp_err_t ret = i2c_del_master_bus(bus[bus_id].bus_handle);
    if (ret == ESP_OK) {
        bus[bus_id].bus_handle = NULL;
        ESP_LOGI(TAG, "I2C bus %d deinitialized", bus_id);
    } else {
        ESP_LOGE(TAG, "Failed to deinit bus %d: %s", bus_id, esp_err_to_name(ret));
    }

    return ret;
}

/**
 * @brief Remove a device from its bus (free resources)
 */
esp_err_t i2c_remove_device(uint8_t dev_id)
{
    if (dev_id >= MAX_DEVICES_NUM || device[dev_id].dev_handle == NULL) {
        return ESP_OK;
    }

    esp_err_t ret = i2c_master_bus_rm_device(device[dev_id].dev_handle);
    if (ret == ESP_OK) {
        device[dev_id].dev_handle = NULL;
        ESP_LOGI(TAG, "Removed device %d (%s)", dev_id, device[dev_id].name ? device[dev_id].name : "?");
    } else {
        ESP_LOGE(TAG, "Failed to remove device %d: %s", dev_id, esp_err_to_name(ret));
    }

    return ret;
}