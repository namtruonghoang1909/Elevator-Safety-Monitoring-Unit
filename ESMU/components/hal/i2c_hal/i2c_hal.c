#include "include/i2c_hal.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"   // for pdMS_TO_TICKS

static const char *TAG = "i2c_hal";

static i2c_bus_info_t    bus[MAX_I2C_BUS_NUM]    = {{0}};
static i2c_device_info_t device[MAX_DEVICES_NUM] = {{0}};

esp_err_t i2c_hal_init(uint8_t bus_id, gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t clk_speed_hz)
{
    if (bus_id >= MAX_I2C_BUS_NUM) {
        ESP_LOGE(TAG, "Invalid bus_id: %d (max %d)", bus_id, MAX_I2C_BUS_NUM - 1);
        return ESP_ERR_INVALID_ARG;
    }

    if (bus[bus_id].bus_handle != NULL) {
        ESP_LOGW(TAG, "I2C bus %d already initialized", bus_id);
        return ESP_OK;
    }

    bus[bus_id].id = bus_id;

    i2c_master_bus_config_t bus_cfg = {
        .clk_source        = I2C_CLK_SRC_DEFAULT,
        .i2c_port          = (i2c_port_t)bus_id,   // I2C_NUM_0 or I2C_NUM_1
        .sda_io_num        = sda_pin,
        .scl_io_num        = scl_pin,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
        .intr_priority     = 0,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_cfg, &bus[bus_id].bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C bus %d: %s", bus_id, esp_err_to_name(ret));
        return ret;
    }

    // Optional: explicitly set speed (new driver allows it)
    if (clk_speed_hz > 0) {
        i2c_master_bus_set_bus_speed(bus[bus_id].bus_handle, clk_speed_hz);
    }

    ESP_LOGI(TAG, "I2C bus %d initialized: SDA=%d, SCL=%d, %lu Hz",
             bus_id, sda_pin, scl_pin, clk_speed_hz);
    return ESP_OK;
}

esp_err_t i2c_hal_add_device(uint8_t dev_id, uint8_t dev_addr, const char *dev_name,
                             uint32_t scl_speed_hz, uint8_t on_bus_id)
{
    if (dev_id >= MAX_DEVICES_NUM) {
        ESP_LOGE(TAG, "Invalid dev_id: %d (max %d)", dev_id, MAX_DEVICES_NUM - 1);
        return ESP_ERR_INVALID_ARG;
    }
    if (on_bus_id >= MAX_I2C_BUS_NUM || bus[on_bus_id].bus_handle == NULL) {
        ESP_LOGE(TAG, "Bus %d not initialized or invalid", on_bus_id);
        return ESP_ERR_INVALID_STATE;
    }
    if (device[dev_id].dev_handle != NULL) {
        ESP_LOGW(TAG, "Device %d already added", dev_id);
        return ESP_OK;
    }

    device[dev_id].id       = dev_id;
    device[dev_id].address  = dev_addr;
    device[dev_id].name     = dev_name;  // just store pointer (caller must keep string alive)

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = dev_addr,
        .scl_speed_hz    = (scl_speed_hz > 0) ? scl_speed_hz : 400000,  // default 400kHz if 0
        .scl_wait_us     = 0,
    };

    esp_err_t ret = i2c_master_bus_add_device(bus[on_bus_id].bus_handle,
                                              &dev_cfg,
                                              &device[dev_id].dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device %d (addr 0x%02X) on bus %d: %s",
                 dev_id, dev_addr, on_bus_id, esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Added device %d: %s @ 0x%02X on bus %d", dev_id, dev_name ? dev_name : "?", dev_addr, on_bus_id);
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
        ESP_LOGE(TAG, "Burst read failed on dev %d: %s",
                 dev_id, esp_err_to_name(ret));
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