/**
 * @file can_platform.h
 * @brief Platform abstraction layer for CAN
 */

#pragma once

#include "stm32f1xx_hal.h"
#include <stdint.h>

/**
 * @brief CAN Platform status codes
 */
typedef enum {
    PLATFORM_CAN_OK = 0,
    PLATFORM_CAN_ERROR,
    PLATFORM_CAN_BUSY,
    PLATFORM_CAN_TIMEOUT
} platform_can_status_t;

/**
 * @brief Initialize the CAN Platform (Filters, Start)
 * @return PLATFORM_CAN_OK on success
 */
platform_can_status_t platform_can_init(void);

/**
 * @brief Send a CAN message (ESMU Protocol compatible)
 * 
 * @param std_id Standard 11-bit ID
 * @param data   Data buffer (max 8 bytes)
 * @param len    Length (0-8)
 * @return platform_can_status_t 
 */
platform_can_status_t platform_can_send(uint32_t std_id, const uint8_t *data, uint8_t len);

/**
 * @brief Internal callback for CAN RX (to be called from HAL callback)
 * 
 * @param std_id Output for received ID
 * @param data   Output buffer for data
 * @param len    Output for length
 * @return PLATFORM_CAN_OK if message was read
 */
platform_can_status_t platform_can_read(uint32_t *std_id, uint8_t *data, uint8_t *len);
