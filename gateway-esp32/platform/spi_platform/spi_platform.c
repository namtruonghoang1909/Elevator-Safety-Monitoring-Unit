#include "spi_platform.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "spi_platform";

static spi_bus_info_t    bus_info[MAX_SPI_BUS_NUM]    = {0};
static spi_device_info_t device_info[MAX_SPI_DEVICES_NUM] = {0};

esp_err_t platform_spi_bus_init(spi_host_device_t host_id, gpio_num_t sclk_pin, gpio_num_t mosi_pin, gpio_num_t miso_pin, spi_dma_chan_t dma_chan)
{
    // Find slot for bus or check if already init
    int slot = -1;
    if (host_id == SPI2_HOST) slot = 0;
    else if (host_id == SPI3_HOST) slot = 1;
    else return ESP_ERR_INVALID_ARG;

    if (bus_info[slot].initialized) {
        ESP_LOGW(TAG, "SPI bus %d already initialized", (int)host_id);
        return ESP_OK;
    }

    spi_bus_config_t bus_cfg = {
        .miso_io_num = miso_pin,
        .mosi_io_num = mosi_pin,
        .sclk_io_num = sclk_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 240 * 240 * 2 // Full frame size
    };

    esp_err_t ret = spi_bus_initialize(host_id, &bus_cfg, dma_chan);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus %d: %s", (int)host_id, esp_err_to_name(ret));
        return ret;
    }

    bus_info[slot].host_id = host_id;
    bus_info[slot].initialized = true;

    ESP_LOGI(TAG, "SPI bus %d initialized (SCK:%d, MOSI:%d, MISO:%d)", (int)host_id, sclk_pin, mosi_pin, miso_pin);
    return ESP_OK;
}

esp_err_t platform_spi_add_device(uint8_t *dev_id_out, spi_host_device_t host_id, gpio_num_t cs_pin, int clock_speed_hz, int mode, const char *dev_name)
{
    if (dev_id_out == NULL) return ESP_ERR_INVALID_ARG;

    int assigned_id = -1;
    for (int i = 0; i < MAX_SPI_DEVICES_NUM; i++) {
        if (device_info[i].dev_handle == NULL) {
            assigned_id = i;
            break;
        }
    }

    if (assigned_id == -1) {
        ESP_LOGE(TAG, "No free SPI device slot");
        return ESP_ERR_NO_MEM;
    }

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = clock_speed_hz,
        .mode = mode,
        .spics_io_num = cs_pin,
        .queue_size = 7,
        .flags = SPI_DEVICE_NO_DUMMY,
    };

    esp_err_t ret = spi_bus_add_device(host_id, &dev_cfg, &device_info[assigned_id].dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
        return ret;
    }

    device_info[assigned_id].id = assigned_id;
    device_info[assigned_id].name = dev_name;

    *dev_id_out = (uint8_t)assigned_id;
    ESP_LOGI(TAG, "Added SPI device %d (%s) on host %d at %d Hz", assigned_id, dev_name ? dev_name : "?", (int)host_id, clock_speed_hz);

    return ESP_OK;
}

esp_err_t platform_spi_transfer(uint8_t dev_id, const uint8_t *data, size_t len)
{
    if (dev_id >= MAX_SPI_DEVICES_NUM || device_info[dev_id].dev_handle == NULL) return ESP_ERR_INVALID_ARG;

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;
    t.tx_buffer = data;

    return spi_device_transmit(device_info[dev_id].dev_handle, &t);
}

esp_err_t platform_spi_transfer_full_duplex(uint8_t dev_id, const uint8_t *tx_data, uint8_t *rx_data, size_t len)
{
    if (dev_id >= MAX_SPI_DEVICES_NUM || device_info[dev_id].dev_handle == NULL) return ESP_ERR_INVALID_ARG;

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;
    t.tx_buffer = tx_data;
    t.rx_buffer = rx_data;

    return spi_device_transmit(device_info[dev_id].dev_handle, &t);
}

esp_err_t platform_spi_queue_transfer(uint8_t dev_id, const uint8_t *data, size_t len)
{
    if (dev_id >= MAX_SPI_DEVICES_NUM || device_info[dev_id].dev_handle == NULL) return ESP_ERR_INVALID_ARG;

    // We need to manage the transaction pointer if we were doing true async with multiple inflight.
    // For simplicity in this Platform Layer, we assume the caller manages the buffer lifetime.
    static spi_transaction_t trans; 
    memset(&trans, 0, sizeof(trans));
    trans.length = len * 8;
    trans.tx_buffer = data;

    return spi_device_queue_trans(device_info[dev_id].dev_handle, &trans, portMAX_DELAY);
}

esp_err_t platform_spi_wait_transfer_done(uint8_t dev_id, uint32_t timeout_ms)
{
    if (dev_id >= MAX_SPI_DEVICES_NUM || device_info[dev_id].dev_handle == NULL) return ESP_ERR_INVALID_ARG;

    spi_transaction_t *ret_trans;
    return spi_device_get_trans_result(device_info[dev_id].dev_handle, &ret_trans, pdMS_TO_TICKS(timeout_ms));
}

esp_err_t platform_spi_remove_device(uint8_t dev_id)
{
    if (dev_id >= MAX_SPI_DEVICES_NUM || device_info[dev_id].dev_handle == NULL) return ESP_OK;

    esp_err_t ret = spi_bus_remove_device(device_info[dev_id].dev_handle);
    if (ret == ESP_OK) {
        device_info[dev_id].dev_handle = NULL;
    }
    return ret;
}

esp_err_t platform_spi_deinit_bus(spi_host_device_t host_id)
{
    int slot = -1;
    if (host_id == SPI2_HOST) slot = 0;
    else if (host_id == SPI3_HOST) slot = 1;
    else return ESP_ERR_INVALID_ARG;

    if (!bus_info[slot].initialized) return ESP_OK;

    esp_err_t ret = spi_bus_free(host_id);
    if (ret == ESP_OK) {
        bus_info[slot].initialized = false;
    }
    return ret;
}
