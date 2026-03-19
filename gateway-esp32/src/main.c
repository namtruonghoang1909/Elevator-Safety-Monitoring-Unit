/**
 * @file main.c
 * @brief ESMU CAN Playground - Minimal Monitor
 */

#include "esp_log.h"
#include "system_config.h"
#include "esmu_protocol.h"
#include "display_service.h"
#include "system_registry.h"
#include "i2c_bsp.h"
#include "can_bsp.h"
#include "motion_proxy.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char *TAG = "CAN_PLAYGROUND";

void app_main(void) {
    ESP_LOGI(TAG, "ESMU CAN Playground Starting...");

    // 1. Initialize Hardware Abstractions
    uint8_t bus_id = 0;
    ESP_ERROR_CHECK(i2c_bsp_bus_init(&bus_id, (gpio_num_t)I2C_SDA_PIN, (gpio_num_t)I2C_SCL_PIN));
    
    // 2. Initialize System Registry (Required for CAN data bridge)
    ESP_ERROR_CHECK(system_registry_init());
    
    // 3. Initialize OLED Display (DISABLED FOR ST7789 MIGRATION)
    // ESP_ERROR_CHECK(display_service_init());
    // system_registry_set_subtext("WAITING FOR CAN...");

    // 4. Initialize CAN (TWAI) @ 500kbps
    can_bsp_config_t can_cfg = {
        .tx_pin = (gpio_num_t)CAN_TX_PIN,
        .rx_pin = (gpio_num_t)CAN_RX_PIN,
        .baud_rate_kbps = CAN_BAUD_RATE_KBPS,
        .mode = CAN_MODE_NORMAL
    };
    
    esp_err_t err = can_bsp_init(&can_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "CAN Init Failed!");
        system_registry_set_subtext("CAN INIT ERROR");
        return;
    }
    
    ESP_ERROR_CHECK(can_bsp_start());
    ESP_LOGI(TAG, "CAN Bus Started at %d kbps", CAN_BAUD_RATE_KBPS);

    // 5. Start Motion Proxy (The CAN listener we updated with Test ID 0x7FF)
    ESP_ERROR_CHECK(motion_proxy_init());

    // 6. Set System State to TESTING (To show subtext on OLED)
    system_registry_set_state(SYSTEM_STATE_TESTING);

    ESP_LOGI(TAG, "Monitor Ready. Observe OLED and Serial Logs.");

    uint32_t uptime = 0;
    while (1) {
        system_registry_update_uptime(uptime++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    }
