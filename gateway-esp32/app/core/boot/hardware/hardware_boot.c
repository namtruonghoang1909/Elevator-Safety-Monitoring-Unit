/**
 * @file hardware_boot.c
 * @brief Boots the hardware related parts of the device
 */

#include "hardware_boot.h"
#include "system_config.h"
#include "board_pins.h"
#include "i2c_platform.h"
#include "spi_platform.h"
#include "can_platform.h"
#include "uart_platform.h"
#include "display.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "SYS_HW";

static esp_err_t init_spiffs(void) {
    ESP_LOGI(TAG, "Initializing SPIFFS...");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}

esp_err_t system_hw_init(void) {
    ESP_LOGI(TAG, "Initializing System Hardware...");

    // 0. Initialize SPIFFS
    init_spiffs();

    // 1. Initialize SPI Bus (for Display)
    esp_err_t ret = platform_spi_bus_init(SPI3_HOST, ST7789_SCL_PIN, ST7789_SDA_PIN, GPIO_NUM_NC, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
    }

    // 2. Initialize Display component
    ret = display_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display init failed: %s", esp_err_to_name(ret));
    } else {
        display_set_brightness(80);
        ESP_LOGI(TAG, "Display component initialized");
    }

    // 3. Initialize I2C Bus
    uint8_t i2c_bus_id = 0;
    ret = platform_i2c_bus_init(&i2c_bus_id, (gpio_num_t)I2C_SDA_PIN, (gpio_num_t)I2C_SCL_PIN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "I2C Bus %d initialized", i2c_bus_id);

    // 2. Initialize CAN Bus (TWAI)
    can_platform_config_t can_cfg = {
        .tx_pin = (gpio_num_t)CAN_TX_PIN,
        .rx_pin = (gpio_num_t)CAN_RX_PIN,
        .baud_rate_kbps = CAN_BAUD_RATE_KBPS,
        .mode = CAN_MODE_NORMAL
    };
    
    ret = platform_can_init(&can_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CAN init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "CAN Bus initialized at %d kbps", CAN_BAUD_RATE_KBPS);

    // 3. Initialize UART Platform (for SIM Module)
    platform_uart_config_t uart_cfg = {
        .port = SIM_UART_PORT,
        .baud_rate = SIM_BAUD_RATE,
        .tx_io_num = SIM_TX_PIN,
        .rx_io_num = SIM_RX_PIN,
        .data_cb = NULL // To be set by SIM driver later
    };

    ret = platform_uart_init(&uart_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART Platform init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "UART Platform initialized for SIM Module");

    return ESP_OK;
}
