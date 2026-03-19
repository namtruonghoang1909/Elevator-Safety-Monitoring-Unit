/**
 * @file bsp_can.h
 * @brief CAN Board Support Package for STM32
 */

#pragma once

#include "stm32f1xx_hal.h"
#include <stdint.h>

/**
 * @brief CAN BSP status codes
 */
typedef enum {
    BSP_CAN_OK = 0,
    BSP_CAN_ERROR,
    BSP_CAN_BUSY
} bsp_can_status_t;

/**
 * @brief Initialize the CAN BSP (Filters, Start)
 * @return BSP_CAN_OK on success
 */
bsp_can_status_t bsp_can_init(void);

/**
 * @brief Send a CAN message (ESMU Protocol compatible)
 * 
 * @param std_id Standard 11-bit ID
 * @param data   Data buffer (max 8 bytes)
 * @param len    Length (0-8)
 * @return bsp_can_status_t 
 */
bsp_can_status_t bsp_can_send(uint32_t std_id, const uint8_t *data, uint8_t len);

/**
 * @brief Internal callback for CAN RX (to be called from HAL callback)
 * 
 * @param std_id Output for received ID
 * @param data   Output buffer for data
 * @param len    Output for length
 * @return BSP_CAN_OK if message was read
 */
bsp_can_status_t bsp_can_read(uint32_t *std_id, uint8_t *data, uint8_t *len);
