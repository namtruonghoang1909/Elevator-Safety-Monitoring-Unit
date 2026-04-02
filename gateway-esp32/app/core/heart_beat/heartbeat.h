/**
 * @file heartbeat.h
 * @brief Visual Heartbeat Service for ESP32 Gateway
 */

#pragma once

#include "esp_err.h"

/**
 * @brief Initialize and start the visual heartbeat task
 * 
 * This task toggles the status LED and updates the system uptime
 * in the registry at a 1Hz frequency.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t heartbeat_init(void);
