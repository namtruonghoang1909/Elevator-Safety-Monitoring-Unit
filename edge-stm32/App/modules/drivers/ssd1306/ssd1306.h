/**
 * @file ssd1306.h
 * @brief SSD1306 OLED display driver (128×64, I2C) for STM32
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "bsp_i2c.h"

// ─────────────────────────────────────────────
// Display Constants
// ─────────────────────────────────────────────

#define SSD1306_I2C_ADDR        0x3C    // 7-bit address
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64
#define SSD1306_PAGES           8       // Height / 8

// Default contrast
#define SSD1306_CONTRAST_DEFAULT    0xCF

// ─────────────────────────────────────────────
// Enums
// ─────────────────────────────────────────────

typedef enum {
    SSD1306_SCROLL_LEFT  = 0x00,
    SSD1306_SCROLL_RIGHT = 0x01
} ssd1306_scroll_dir_t;

// ─────────────────────────────────────────────
// Data Structures
// ─────────────────────────────────────────────

typedef struct {
    uint8_t  address;               // I2C slave address (usually 0x3C)
    uint8_t  contrast;              // Initial contrast (0-255)
} ssd1306_config_t;

// ─────────────────────────────────────────────
// Core APIs
// ─────────────────────────────────────────────

bsp_i2c_status_t ssd1306_init(const ssd1306_config_t *cfg);
bsp_i2c_status_t ssd1306_display_on(void);
bsp_i2c_status_t ssd1306_display_off(void);
bsp_i2c_status_t ssd1306_set_contrast(uint8_t contrast);
bsp_i2c_status_t ssd1306_invert_display(bool invert);
bsp_i2c_status_t ssd1306_clear(void);
bsp_i2c_status_t ssd1306_fill_screen(uint8_t color);
bsp_i2c_status_t ssd1306_set_cursor(uint8_t page, uint8_t col);
bsp_i2c_status_t ssd1306_write_data(const uint8_t *data, size_t len);
bsp_i2c_status_t ssd1306_write_page(uint8_t page, const uint8_t *data);
bsp_i2c_status_t ssd1306_write_char(char c, uint8_t page, uint8_t col);
bsp_i2c_status_t ssd1306_write_string(const char *str, uint8_t page, uint8_t col);
