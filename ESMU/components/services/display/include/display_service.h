/**
 * @file display_service.h
 * @brief High-level UI management for SSD1306
 */

#pragma once

#include "esp_err.h"
#include <stdint.h>

/**
 * @brief Initialize the Display Service
 * 
 * Sets up SSD1306 hardware, initializes the system registry (if needed),
 * and starts the background refresh task (15Hz).
 * 
 * @return ESP_OK on success
 */
esp_err_t display_service_init(void);

/**
 * @brief Deinitialize the Display Service and stop refresh task
 * @return ESP_OK on success
 */
esp_err_t display_service_deinit(void);
