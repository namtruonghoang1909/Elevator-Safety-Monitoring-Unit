/**
 * @file can_bsp.c
 * @brief ESP32 TWAI (CAN) Board Support Package Implementation
 */

#include "can_bsp.h"
#include "esp_log.h"
#include "esp_rom_gpio.h"
#include "soc/gpio_sig_map.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

// ─────────────────────────────────────────────
// Private variables, macros
// ─────────────────────────────────────────────

static const char *TAG = "can_bsp";

static SemaphoreHandle_t s_tx_mutex = NULL;
static bool s_is_initialized = false;
static can_mode_t s_mode = CAN_MODE_NORMAL;

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

static esp_err_t get_timing_config(uint32_t baud_kbps, twai_timing_config_t *out_timing) {
    if (!out_timing) return ESP_ERR_INVALID_ARG;

    switch (baud_kbps) {
        case 125:  *out_timing = (twai_timing_config_t)TWAI_TIMING_CONFIG_125KBITS(); break;
        case 250:  *out_timing = (twai_timing_config_t)TWAI_TIMING_CONFIG_250KBITS(); break;
        case 500:  *out_timing = (twai_timing_config_t)TWAI_TIMING_CONFIG_500KBITS(); break;
        case 1000: *out_timing = (twai_timing_config_t)TWAI_TIMING_CONFIG_1MBITS();   break;
        default:
            ESP_LOGE(TAG, "Unsupported baud rate: %ld kbps", baud_kbps);
            return ESP_ERR_NOT_SUPPORTED;
    }
    return ESP_OK;
}

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

esp_err_t can_bsp_init(const can_bsp_config_t *cfg) {
    if (s_is_initialized) return ESP_ERR_INVALID_STATE;
    if (!cfg) return ESP_ERR_INVALID_ARG;

    ESP_LOGI(TAG, "Initializing CAN (TWAI) at %ld kbps (TX:%d, RX:%d)", 
             cfg->baud_rate_kbps, cfg->tx_pin, cfg->rx_pin);

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(cfg->tx_pin, cfg->rx_pin, TWAI_MODE_NORMAL);
    
    if (cfg->mode == CAN_MODE_NO_ACK || cfg->mode == CAN_MODE_LOOPBACK) {
        g_config.mode = TWAI_MODE_NO_ACK;
    }

    twai_timing_config_t t_config;
    esp_err_t err = get_timing_config(cfg->baud_rate_kbps, &t_config);
    if (err != ESP_OK) return err;

    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(err));
        return err;
    }

    twai_reconfigure_alerts(TWAI_ALERT_TX_SUCCESS | TWAI_ALERT_TX_FAILED | 
                            TWAI_ALERT_ARB_LOST   | TWAI_ALERT_BUS_ERROR |
                            TWAI_ALERT_RX_DATA, NULL);

    gpio_set_pull_mode(cfg->rx_pin, GPIO_PULLUP_ONLY);

    if (cfg->mode == CAN_MODE_LOOPBACK) {
        ESP_LOGI(TAG, "Configuring Internal GPIO Matrix Loopback (TX -> RX)");
        esp_rom_gpio_connect_in_signal(cfg->tx_pin, TWAI_RX_IDX, false);
    }

    if (s_tx_mutex == NULL) {
        s_tx_mutex = xSemaphoreCreateMutex();
    }

    s_mode = cfg->mode;
    s_is_initialized = true;
    ESP_LOGI(TAG, "CAN BSP initialized successfully");
    return ESP_OK;
}

esp_err_t can_bsp_start(void) {
    if (!s_is_initialized) return ESP_ERR_INVALID_STATE;
    
    esp_err_t err = twai_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI bus: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t can_bsp_stop(void) {
    if (!s_is_initialized) return ESP_ERR_INVALID_STATE;
    
    esp_err_t err = twai_stop();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop TWAI bus: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t can_bsp_transmit(uint32_t id, const uint8_t *data, uint8_t len) {
    if (!s_is_initialized || !s_tx_mutex) return ESP_ERR_INVALID_STATE;
    if (len > 8) return ESP_ERR_INVALID_ARG;

    twai_message_t message = {0};
    message.identifier = id;
    message.data_length_code = len;
    message.extd = 0;
    message.rtr = 0;
    message.ss = 0;
    message.self = (s_mode == CAN_MODE_LOOPBACK) ? 1 : 0; 
    
    if (data && len > 0) {
        memcpy(message.data, data, len);
    }

    xSemaphoreTake(s_tx_mutex, portMAX_DELAY);
    esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(100));
    xSemaphoreGive(s_tx_mutex);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transmit frame (ID:0x%lX): %s", id, esp_err_to_name(err));
    }
    return err;
}

esp_err_t can_bsp_receive(uint32_t *id_out, uint8_t *data_out, uint8_t *len_out, uint32_t timeout_ms) {
    if (!s_is_initialized) return ESP_ERR_INVALID_STATE;
    if (!id_out || !data_out || !len_out) return ESP_ERR_INVALID_ARG;

    twai_message_t message = {0};
    esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(timeout_ms));
    
    if (err == ESP_OK) {
        *id_out = message.identifier;
        *len_out = message.data_length_code;
        memcpy(data_out, message.data, message.data_length_code);
    }
    
    return err;
}

esp_err_t can_bsp_get_status(twai_status_info_t *out_status) {
    if (!s_is_initialized || !out_status) return ESP_ERR_INVALID_ARG;
    return twai_get_status_info(out_status);
}

esp_err_t can_bsp_deinit(void) {
    if (!s_is_initialized) return ESP_OK;

    twai_stop(); 
    esp_err_t err = twai_driver_uninstall();
    
    if (err == ESP_OK) {
        s_is_initialized = false;
        ESP_LOGI(TAG, "CAN BSP deinitialized");
    } else {
        ESP_LOGE(TAG, "Failed to uninstall TWAI driver: %s", esp_err_to_name(err));
    }
    
    return err;
}
