/**
 * @file can_platform.h
 * @brief STM32 CAN Platform Abstraction Layer (HAL-based)
 *
 * Provides a thread-safe, simplified API for CAN communication on STM32.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

/**
 * @brief CAN platform return codes (to match ESP32 esp_err_t style)
 */
typedef int can_err_t;
#define CAN_OK          0
#define CAN_FAIL        -1
#define CAN_ERR_TIMEOUT -2

/**
 * @brief Operational modes for the CAN bus
 */
typedef enum {
    CAN_MODE_NORMAL,      /**< Standard CAN mode */
    CAN_MODE_LOOPBACK     /**< Internal loopback mode */
} can_mode_t;

/**
 * @brief CAN platform configuration
 */
typedef struct {
    uint32_t baud_rate_kbps;  /**< Bus speed (e.g., 500, 250) */
    can_mode_t mode;          /**< Mode: Normal, Loopback */
} can_config_t;

/**
 * @brief Initialize the CAN driver (wraps HAL_CAN_Init)
 */
can_err_t can_init(const can_config_t *cfg);

/**
 * @brief Start the CAN bus (Activate interrupts/filters)
 */
can_err_t can_start(void);

/**
 * @brief Stop the CAN bus
 */
can_err_t can_stop(void);

/**
 * @brief Send a standard CAN frame (Thread-safe)
 */
can_err_t can_transmit(uint32_t id, const uint8_t *data, uint8_t len);

/**
 * @brief Receive a single standard CAN frame (Blocking with timeout)
 */
can_err_t can_receive(uint32_t *id_out, uint8_t *data_out, uint8_t *len_out, uint32_t timeout_ms);

/**
 * @brief Deinitialize CAN driver
 */
can_err_t can_deinit(void);
