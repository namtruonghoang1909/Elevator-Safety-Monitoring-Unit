/**
 * @file hardware_boot.h
 * @brief Boots the hardware related parts of the device
 */

#pragma once

#include "esp_err.h"

/**
 * @brief Initialize all system hardware (Buses, Pins)
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t system_hw_init(void);
