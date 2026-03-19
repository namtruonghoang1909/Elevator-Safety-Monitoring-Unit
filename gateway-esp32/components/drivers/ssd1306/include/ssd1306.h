/**
 * @file ssd1306.h
 * @brief SSD1306 OLED display driver (128×64, I2C)
 *
 * Built on top of i2c_bsp.
 * Provides low-level display control and mid-level APIs for
 * higher-level code to build graphics on top.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "i2c_bsp.h"

// ─────────────────────────────────────────────
// Display Constants
// ─────────────────────────────────────────────

#define SSD1306_I2C_ADDR        0x3C    // 7-bit address (0x78 = 0x3C << 1)
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64
#define SSD1306_PAGES           8       // Height / 8 (each page = 8 rows)

// Default contrast
#define SSD1306_CONTRAST_DEFAULT    0xCF

// ─────────────────────────────────────────────
// Enums
// ─────────────────────────────────────────────

/**
 * @brief Scroll direction
 */
typedef enum {
    SSD1306_SCROLL_LEFT  = 0x00,
    SSD1306_SCROLL_RIGHT = 0x01
} ssd1306_scroll_dir_t;

// ─────────────────────────────────────────────
// Data Structures
// ─────────────────────────────────────────────

/**
 * @brief Configuration structure for ssd1306_init()
 */
typedef struct {
    uint8_t  address;               // I2C slave address (0x3C or 0x78)
    const char *name;                // Device name for logging
    uint32_t scl_speed_hz;           // I2C clock speed (Hz)
    uint8_t  bus_id;                // Bus ID from i2c_bus_init()
    uint8_t  contrast;              // Initial contrast (0-255, default 0xCF)
} ssd1306_config_t;

// ─────────────────────────────────────────────
// Core APIs
// ─────────────────────────────────────────────

/**
 * @brief Initialize SSD1306 display
 *
 * Registers device on I2C bus and initializes display.
 *
 * @param cfg         Configuration (address, name, speed, bus_id)
 * @param dev_id_out  [out] Assigned device ID
 * @return ESP_OK on success
 */
esp_err_t ssd1306_init(const ssd1306_config_t *cfg, uint8_t *dev_id_out);

/**
 * @brief Deinitialize SSD1306 display and remove from I2C bus
 *
 * @param dev_id Device ID
 * @return ESP_OK on success
 */
esp_err_t ssd1306_deinit(uint8_t dev_id);

// ─────────────────────────────────────────────
// Display Control
// ─────────────────────────────────────────────

/**
 * @brief Turn display ON
 */
esp_err_t ssd1306_display_on(uint8_t dev_id);

/**
 * @brief Turn display OFF
 */
esp_err_t ssd1306_display_off(uint8_t dev_id);

/**
 * @brief Set display contrast/brightness
 *
 * @param dev_id   Device ID
 * @param contrast Contrast level (0-255, 0x00 = dim, 0xFF = bright)
 * @return ESP_OK on success
 */
esp_err_t ssd1306_set_contrast(uint8_t dev_id, uint8_t contrast);

/**
 * @brief Invert display colors
 *
 * @param dev_id   Device ID
 * @param invert   true = inverted, false = normal
 * @return ESP_OK on success
 */
esp_err_t ssd1306_invert_display(uint8_t dev_id, bool invert);

// ─────────────────────────────────────────────
// Display Content
// ─────────────────────────────────────────────

/**
 * @brief Clear the entire display (fill GDDRAM with 0x00)
 */
esp_err_t ssd1306_clear(uint8_t dev_id);

/**
 * @brief Fill entire display with a color
 *
 * @param dev_id  Device ID
 * @param color   0x00 = black, 0xFF = white
 * @return ESP_OK on success
 */
esp_err_t ssd1306_fill_screen(uint8_t dev_id, uint8_t color);

/**
 * @brief Set cursor position for next write
 *
 * @param dev_id  Device ID
 * @param page    Page number (0-7)
 * @param col     Column number (0-127)
 * @return ESP_OK on success
 */
esp_err_t ssd1306_set_cursor(uint8_t dev_id, uint8_t page, uint8_t col);

/**
 * @brief Write raw pixel data to GDDRAM at current cursor
 *
 * Call ssd1306_set_cursor() first to position the cursor.
 *
 * @param dev_id  Device ID
 * @param data    Pixel bytes (each byte = 8 vertical pixels)
 * @param len     Number of bytes (max 128)
 * @return ESP_OK on success
 */
esp_err_t ssd1306_write_data(uint8_t dev_id, const uint8_t *data, size_t len);

/**
 * @brief Write a single page row (128 pixels horizontally)
 *
 * Convenience function that handles cursor setting internally.
 *
 * @param dev_id  Device ID
 * @param page    Page number (0-7)
 * @param data    128 bytes of pixel data
 * @return ESP_OK on success
 */
esp_err_t ssd1306_write_page(uint8_t dev_id, uint8_t page, const uint8_t *data);

// ─────────────────────────────────────────────
// Scrolling
// ─────────────────────────────────────────────

/**
 * @brief Horizontally scroll the display content
 *
 * @param dev_id     Device ID
 * @param direction  Scroll direction (SSD1306_SCROLL_LEFT or RIGHT)
 * @param start_page Start page (0-7)
 * @param end_page   End page (0-7)
 * @param speed      Scroll speed (0x00-0x07, 0x00 = fastest, 0x07 = slowest)
 * @return ESP_OK on success
 */
esp_err_t ssd1306_scroll_horizontal(uint8_t dev_id, ssd1306_scroll_dir_t direction,
                                    uint8_t start_page, uint8_t end_page, uint8_t speed);

/**
 * @brief Stop horizontal scrolling
 *
 * @param dev_id Device ID
 * @return ESP_OK on success
 */
esp_err_t ssd1306_scroll_stop(uint8_t dev_id);
