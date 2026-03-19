/**
 * @file can_bsp.h
 * @brief ESP32 TWAI (CAN) Board Support Package Abstraction Layer
 *
 * Provides a thread-safe, simplified API for bus initialization and 
 * frame exchange on the ESP32 Gateway.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "driver/twai.h"
#include "driver/gpio.h"
#include "esp_err.h"

/**
 * @brief Operational modes for the CAN bus
 */
typedef enum {
    CAN_MODE_NORMAL,      /**< Standard CAN mode (Read/Write with ACK) */
    CAN_MODE_NO_ACK,      /**< Standard mode but doesn't require ACKs (Listen Only + Self Test) */
    CAN_MODE_LOOPBACK     /**< Internal loopback for driver verification */
} can_mode_t;

/**
 * @brief CAN BSP configuration
 */
typedef struct {
    gpio_num_t tx_pin;          /**< TWAI TX GPIO pin */
    gpio_num_t rx_pin;          /**< TWAI RX GPIO pin */
    uint32_t   baud_rate_kbps;  /**< Bus speed (e.g., 500, 250). Default 500. */
    can_mode_t mode;            /**< Mode: Normal, Loopback, etc. */
} can_bsp_config_t;

/**
 * @brief Initialize the CAN (TWAI) driver and configure hardware
 */
esp_err_t can_bsp_init(const can_bsp_config_t *cfg);

/**
 * @brief Transition the bus to the "Running" state
 */
esp_err_t can_bsp_start(void);

/**
 * @brief Transition the bus to the "Stopped" state
 */
esp_err_t can_bsp_stop(void);

/**
 * @brief Send a standard CAN frame (Thread-safe)
 */
esp_err_t can_bsp_transmit(uint32_t id, const uint8_t *data, uint8_t len);

/**
 * @brief Receive a single standard CAN frame (Blocking with timeout)
 */
esp_err_t can_bsp_receive(uint32_t *id_out, uint8_t *data_out, uint8_t *len_out, uint32_t timeout_ms);

/**
 * @brief Get current TWAI bus status
 */
esp_err_t can_bsp_get_status(twai_status_info_t *out_status);

/**
 * @brief Deinitialize and uninstall the CAN driver
 */
esp_err_t can_bsp_deinit(void);
