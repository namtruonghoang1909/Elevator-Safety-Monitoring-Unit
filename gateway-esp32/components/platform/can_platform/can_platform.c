/**
 * @file can_platform.c
 * @brief ESP32 TWAI (CAN) Platform Implementation
 *
 * This implementation provides a thread-safe wrapper around the ESP-IDF TWAI driver.
 * It manages the driver lifecycle (init, start, stop, deinit) and provides
 * simplified transmit/receive functions for standard 11-bit CAN frames.
 */

#include "can_platform.h"
#include "esp_log.h"
#include "esp_rom_gpio.h"
#include "soc/gpio_sig_map.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

// ─────────────────────────────────────────────
// Private variables, macros
// ─────────────────────────────────────────────

static const char *TAG = "CAN_PLATFORM";

/** @brief Mutex to ensure thread-safe access to the TWAI transmit function */
static SemaphoreHandle_t s_tx_mutex = NULL;

/** @brief Internal state tracking to prevent multiple initializations */
static bool s_is_initialized = false;

/** @brief Track the current operating mode for specialized behavior (e.g., self-test) */
static can_mode_t s_mode = CAN_MODE_NORMAL;

// ─────────────────────────────────────────────
// Private helpers declarations or definitions
// ─────────────────────────────────────────────

/**
 * @brief Map a baud rate in kbps to the corresponding TWAI timing configuration.
 *
 * @param baud_kbps The desired baud rate (125, 250, 500, 1000).
 * @param out_timing Pointer to store the resulting TWAI timing config.
 * @return 
 *  - ESP_OK: Success.
 *  - ESP_ERR_INVALID_ARG: NULL pointer provided.
 *  - ESP_ERR_NOT_SUPPORTED: Baud rate not in the supported list.
 */
static esp_err_t get_timing_config(uint32_t baud_kbps, twai_timing_config_t *out_timing) {
    if (!out_timing) return ESP_ERR_INVALID_ARG;

    // Map common baud rates to predefined ESP-IDF timing macros
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
// Global function declarations or definitions
// ─────────────────────────────────────────────

/**
 * @brief Initialize the CAN (TWAI) driver and configure hardware.
 * 
 * Must be called before any other can_* functions.
 * Sets up GPIO pins, timing, and installs the driver.
 *
 * @param cfg Pointer to the configuration structure.
 * @return 
 *  - ESP_OK: Successfully initialized.
 *  - ESP_ERR_INVALID_STATE: Driver already initialized.
 *  - ESP_ERR_INVALID_ARG: NULL config provided.
 *  - ESP_FAIL: Driver installation failed.
 */
esp_err_t can_init(const can_config_t *cfg) {
    // Check if already initialized to prevent resource leaks
    if (s_is_initialized) return ESP_ERR_INVALID_STATE;
    if (!cfg) return ESP_ERR_INVALID_ARG;

    ESP_LOGI(TAG, "Initializing CAN (TWAI) at %ld kbps (TX:%d, RX:%d)", 
             cfg->baud_rate_kbps, cfg->tx_pin, cfg->rx_pin);

    // Prepare general configuration (mode, pins, queues)
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(cfg->tx_pin, cfg->rx_pin, TWAI_MODE_NORMAL);
    
    // Configure specific operational modes
    if (cfg->mode == CAN_MODE_NO_ACK || cfg->mode == CAN_MODE_LOOPBACK) {
        g_config.mode = TWAI_MODE_NO_ACK;
    }

    // Map baud rate to timing parameters
    twai_timing_config_t t_config;
    esp_err_t err = get_timing_config(cfg->baud_rate_kbps, &t_config);
    if (err != ESP_OK) return err;

    // Set filter to accept all standard 11-bit frames by default
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install the driver into the system
    err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(err));
        return err;
    }

    // Configure alerts to monitor bus health and frame arrival
    twai_reconfigure_alerts(TWAI_ALERT_TX_SUCCESS | TWAI_ALERT_TX_FAILED | 
                            TWAI_ALERT_ARB_LOST   | TWAI_ALERT_BUS_ERROR |
                            TWAI_ALERT_RX_DATA, NULL);

    // Explicitly set pull-up on RX pin to ensure it stays recessive (High) when idle.
    // This can prevent the controller from seeing "phantom" dominant bits if the pin is floating.
    gpio_set_pull_mode(cfg->rx_pin, GPIO_PULLUP_ONLY);

    // If in Loopback mode, route the TX peripheral signal back to the RX peripheral input 
    // internally using the GPIO Matrix. This allows testing without an external transceiver/wire.
    if (cfg->mode == CAN_MODE_LOOPBACK) {
        ESP_LOGI(TAG, "Configuring Internal GPIO Matrix Loopback (TX -> RX)");
        esp_rom_gpio_connect_in_signal(cfg->tx_pin, TWAI_RX_IDX, false);
    }

    // Create the mutex for thread-safe transmission
    if (s_tx_mutex == NULL) {
        s_tx_mutex = xSemaphoreCreateMutex();
    }

    s_mode = cfg->mode;
    s_is_initialized = true;
    ESP_LOGI(TAG, "CAN platform initialized successfully");
    return ESP_OK;
}

/**
 * @brief Transition the bus to the "Running" state.
 * 
 * Driver must be initialized first.
 *
 * @return 
 *  - ESP_OK: Bus started.
 *  - ESP_ERR_INVALID_STATE: Driver not initialized or already running.
 */
esp_err_t can_start(void) {
    if (!s_is_initialized) return ESP_ERR_INVALID_STATE;
    
    esp_err_t err = twai_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI bus: %s", esp_err_to_name(err));
    }
    return err;
}

/**
 * @brief Transition the bus to the "Stopped" state.
 * 
 * Driver must be in Running state.
 *
 * @return 
 *  - ESP_OK: Bus stopped.
 *  - ESP_ERR_INVALID_STATE: Driver not initialized.
 */
esp_err_t can_stop(void) {
    if (!s_is_initialized) return ESP_ERR_INVALID_STATE;
    
    esp_err_t err = twai_stop();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop TWAI bus: %s", esp_err_to_name(err));
    }
    return err;
}

/**
 * @brief Send a standard CAN frame (Thread-safe).
 * 
 * Automatically handles standard frame formatting and non-RTR flags.
 * Uses a mutex to allow multiple tasks to share the same bus.
 *
 * @param id The standard 11-bit CAN ID (0-0x7FF).
 * @param data Data payload buffer.
 * @param len Payload length (0-8).
 * @return 
 *  - ESP_OK: Frame queued for transmission.
 *  - ESP_ERR_INVALID_ARG: Data too long (>8 bytes).
 *  - ESP_ERR_INVALID_STATE: Driver not started.
 */
esp_err_t can_transmit(uint32_t id, const uint8_t *data, uint8_t len) {
    if (!s_is_initialized || !s_tx_mutex) return ESP_ERR_INVALID_STATE;
    if (len > 8) return ESP_ERR_INVALID_ARG;

    // Prepare the TWAI message structure (Zero-initialize all fields)
    twai_message_t message = {0};
    message.identifier = id;
    message.data_length_code = len;
    message.extd = 0; // Standard 11-bit ID
    message.rtr = 0;  // Not a Remote Transmission Request
    message.ss = 0;   // No Single Shot
    message.self = (s_mode == CAN_MODE_LOOPBACK) ? 1 : 0; 
    message.dlc_non_comp = 0;
    
    if (data && len > 0) {
        memcpy(message.data, data, len);
    }

    // Protect transmission with a mutex
    xSemaphoreTake(s_tx_mutex, portMAX_DELAY);
    esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(100));
    xSemaphoreGive(s_tx_mutex);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transmit frame (ID:0x%lX): %s", id, esp_err_to_name(err));
    }
    return err;
}

/**
 * @brief Receive a single standard CAN frame (Blocking with timeout).
 * 
 * @param id_out Pointer to store the received CAN ID.
 * @param data_out Buffer to store the received data (min 8 bytes).
 * @param len_out Pointer to store the received data length.
 * @param timeout_ms Maximum time to wait for a frame (ms).
 * @return 
 *  - ESP_OK: Frame received successfully.
 *  - ESP_ERR_TIMEOUT: No frame arrived within timeout_ms.
 *  - ESP_ERR_INVALID_ARG: NULL pointers provided.
 */
esp_err_t can_receive(uint32_t *id_out, uint8_t *data_out, uint8_t *len_out, uint32_t timeout_ms) {
    if (!s_is_initialized) return ESP_ERR_INVALID_STATE;
    if (!id_out || !data_out || !len_out) return ESP_ERR_INVALID_ARG;

    twai_message_t message = {0};
    esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(timeout_ms));
    
    if (err == ESP_OK) {
        // Unpack the TWAI message into simple output types
        *id_out = message.identifier;
        *len_out = message.data_length_code;
        memcpy(data_out, message.data, message.data_length_code);
    }
    
    return err;
}

esp_err_t can_get_status(twai_status_info_t *out_status) {
    if (!s_is_initialized || !out_status) return ESP_ERR_INVALID_ARG;
    return twai_get_status_info(out_status);
}

/**
 * @brief Deinitialize and uninstall the CAN driver.
 * 
 * Frees all hardware and software resources allocated for CAN.
 *
 * @return 
 *  - ESP_OK: Successfully uninstalled.
 *  - ESP_FAIL: Driver uninstall failed.
 */
esp_err_t can_deinit(void) {
    if (!s_is_initialized) return ESP_OK;

    // Bus must be in 'Stopped' state to uninstall
    twai_stop(); 
    esp_err_t err = twai_driver_uninstall();
    
    if (err == ESP_OK) {
        s_is_initialized = false;
        ESP_LOGI(TAG, "CAN platform deinitialized");
    } else {
        ESP_LOGE(TAG, "Failed to uninstall TWAI driver: %s", esp_err_to_name(err));
    }
    
    return err;
}
