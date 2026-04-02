#include "can_platform.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "can_platform";
static SemaphoreHandle_t tx_mutex = NULL;

esp_err_t platform_can_init(const can_platform_config_t *cfg)
{
    if (cfg == NULL) return ESP_ERR_INVALID_ARG;

    if (tx_mutex == NULL) {
        tx_mutex = xSemaphoreCreateMutex();
    }

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(cfg->tx_pin, cfg->rx_pin, TWAI_MODE_NORMAL);
    
    if (cfg->mode == CAN_MODE_LOOPBACK) {
        g_config.mode = TWAI_MODE_NO_ACK;
    } else if (cfg->mode == CAN_MODE_NO_ACK) {
        g_config.mode = TWAI_MODE_NO_ACK;
    }

    twai_timing_config_t t_config;
    switch (cfg->baud_rate_kbps) {
        case 1000: t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_1MBITS(); break;
        case 500:  t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_500KBITS(); break;
        case 250:  t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_250KBITS(); break;
        case 125:  t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_125KBITS(); break;
        default:
            ESP_LOGE(TAG, "Unsupported baud rate: %d kbps. Defaulting to 500k", (int)cfg->baud_rate_kbps);
            t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_500KBITS();
            break;
    }

    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "CAN driver installed (TX:%d, RX:%d, %d kbps)", cfg->tx_pin, cfg->rx_pin, (int)cfg->baud_rate_kbps);
    } else {
        ESP_LOGE(TAG, "Failed to install CAN driver: %s", esp_err_to_name(ret));
    }

    return ret;
}

esp_err_t platform_can_start(void)
{
    esp_err_t ret = twai_start();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "CAN bus started");
    }
    return ret;
}

esp_err_t platform_can_stop(void)
{
    return twai_stop();
}

esp_err_t platform_can_transmit(uint32_t id, const uint8_t *data, uint8_t len)
{
    if (len > 8) return ESP_ERR_INVALID_ARG;

    twai_message_t message;
    message.identifier = id;
    message.extd = 0; // Standard frame
    message.data_length_code = len;
    for (int i = 0; i < len; i++) {
        message.data[i] = data[i];
    }

    if (xSemaphoreTake(tx_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t ret = twai_transmit(&message, pdMS_TO_TICKS(10));
    xSemaphoreGive(tx_mutex);

    return ret;
}

esp_err_t platform_can_receive(uint32_t *id_out, uint8_t *data_out, uint8_t *len_out, uint32_t timeout_ms)
{
    twai_message_t message;
    esp_err_t ret = twai_receive(&message, pdMS_TO_TICKS(timeout_ms));
    
    if (ret == ESP_OK) {
        *id_out = message.identifier;
        *len_out = message.data_length_code;
        for (int i = 0; i < message.data_length_code; i++) {
            data_out[i] = message.data[i];
        }
    }
    return ret;
}

esp_err_t platform_can_get_status(twai_status_info_t *out_status)
{
    return twai_get_status_info(out_status);
}

esp_err_t platform_can_deinit(void)
{
    twai_stop();
    esp_err_t ret = twai_driver_uninstall();
    if (tx_mutex) {
        vSemaphoreDelete(tx_mutex);
        tx_mutex = NULL;
    }
    return ret;
}
