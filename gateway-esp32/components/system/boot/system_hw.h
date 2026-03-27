/**
 * @file system_hw.h
 * @brief Hardware Abstraction Layer for ESP32 Gateway
 */

#pragma once

#include "esp_err.h"

/**
 * @brief Initialize all system hardware (Buses, Pins)
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t system_hw_init(void);
