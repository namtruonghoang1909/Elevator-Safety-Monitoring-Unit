#include "include/i2c_hal.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_TIMEOUT_MS      100
#define INVALID_ID_MARKER   255

static const char *TAG = "i2c_hal";

static i2c_bus_info_t    bus[MAX_I2C_BUS_NUM]    = {{0}};
static i2c_device_info_t device[MAX_DEVICES_NUM] = {{0}};

esp_err_t i2c_hal_init(uint8_t *bus_id_out, gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t clk_speed_hz)
{
    if (bus_id_out == NULL) {
        ESP_LOGE(TAG, "bus_id_out pointer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t requested = *bus_id_out;
    uint8_t assigned  = INVALID_ID_MARKER;

    // Auto-assign if caller passed invalid value
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

    // Already initialized?
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

    if (clk_speed_hz > 0) {
        ret = i2c_master_bus_set_bus_speed(bus[assigned].bus_handle, clk_speed_hz);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set bus speed %lu Hz: %s", clk_speed_hz, esp_err_to_name(ret));
            i2c_del_master_bus(bus[assigned].bus_handle);
            bus[assigned].bus_handle = NULL;
            return ret;
        }
    }

    ESP_LOGI(TAG, "I2C bus %d initialized: SDA=%d, SCL=%d, %lu Hz",
             assigned, sda_pin, scl_pin, clk_speed_hz ? clk_speed_hz : 100000);

    *bus_id_out = assigned;
    return ESP_OK;
}

esp_err_t i2c_hal_add_device(uint8_t *dev_id_out,
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
esp_err_t i2c_hal_write_reg(uint8_t dev_id, uint8_t reg, uint8_t val)
{
    if (dev_id >= MAX_DEVICES_NUM || device[dev_id].dev_handle == NULL) {
        ESP_LOGE(TAG, "Device %d not initialized", dev_id);
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t buf[2] = {reg, val};
    esp_err_t ret = i2c_master_transmit(device[dev_id].dev_handle, buf, 2, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Write reg failed on dev %d: %s", dev_id, esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t i2c_hal_read_reg(uint8_t dev_id, uint8_t reg, uint8_t *val)
{
    if (dev_id >= MAX_DEVICES_NUM || device[dev_id].dev_handle == NULL || val == NULL) {
        ESP_LOGE(TAG, "Invalid params for read on dev %d", dev_id);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = i2c_master_transmit_receive(device[dev_id].dev_handle,
                                                &reg ,1,
                                                val, 1,
                                                pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read reg failed on dev %d: %s", dev_id, esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t i2c_hal_read_consecutive_regs(uint8_t dev_id,
                                        const uint8_t start_reg,
                                        uint8_t *data_buf, size_t buf_len)
{
    if (dev_id >= MAX_DEVICES_NUM || device[dev_id].dev_handle == NULL) {
        ESP_LOGE(TAG, "Device %d not initialized", dev_id);
        return ESP_ERR_INVALID_STATE;
    }
    if (data_buf == NULL || buf_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t reg = start_reg;
    esp_err_t ret = i2c_master_transmit_receive(
            device[dev_id].dev_handle,
            &reg, 1,
            data_buf, buf_len,
            pdMS_TO_TICKS(I2C_TIMEOUT_MS));

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Burst read failed on dev %d: %s", dev_id, esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t i2c_hal_deinit_bus(uint8_t bus_id)
{
    if (bus_id >= MAX_I2C_BUS_NUM || bus[bus_id].bus_handle == NULL) {
        return ESP_OK;
    }

    // Optional: remove all devices first (in real code, loop over devices on this bus)
    esp_err_t ret = i2c_del_master_bus(bus[bus_id].bus_handle);
    if (ret == ESP_OK) {
        bus[bus_id].bus_handle = NULL;
        ESP_LOGI(TAG, "I2C bus %d deinitialized", bus_id);
    } else {
        ESP_LOGE(TAG, "Failed to deinit bus %d: %s", bus_id, esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t i2c_hal_remove_device(uint8_t dev_id)
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