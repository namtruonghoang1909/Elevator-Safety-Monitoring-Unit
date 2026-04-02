/**
 * @file hardware_boot.c
 * @brief Boots the hardware related parts of the device
 */

#include "hardware_boot.h"
#include "system_config.h"
#include "board_pins.h"
#include "i2c_platform.h"
#include "can_platform.h"
#include "uart_platform.h"
#include "esp_log.h"

static const char *TAG = "SYS_HW";

esp_err_t system_hw_init(void) {
    ESP_LOGI(TAG, "Initializing System Hardware...");

    // 1. Initialize I2C Bus
    uint8_t i2c_bus_id = 0;
    esp_err_t ret = platform_i2c_bus_init(&i2c_bus_id, (gpio_num_t)I2C_SDA_PIN, (gpio_num_t)I2C_SCL_PIN);
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
