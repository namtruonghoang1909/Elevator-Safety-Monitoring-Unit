/**
 * @file display.h
 * @brief High-level service for managing the ST7789 TFT display
 */

#pragma once

#include "esp_err.h"
#include <stdbool.h>

/**
 * @brief Initialize the display component
 * @return ESP_OK on success
 */
esp_err_t display_init(void);

/**
 * @brief Start the display refresh task
 * @return ESP_OK on success
 */
esp_err_t display_start(void);

/**
 * @brief Set display brightness
 * @param percent 0-100
 */
void display_set_brightness(uint8_t percent);

/**
 * @brief Force a complete screen refresh
 */
void display_refresh(void);
