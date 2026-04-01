/**
 * @file st7789.h
 * @brief ST7789 TFT display driver (240x240, SPI)
 *
 * Built on top of spi_bsp and pwm_bsp.
 * Supports RGB565 color format and high-speed DMA transfers.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "spi_bsp.h"
#include "st7789_fonts.h"

// ─────────────────────────────────────────────
// Display Constants
// ─────────────────────────────────────────────

#define ST7789_WIDTH            240
#define ST7789_HEIGHT           240

// Common Colors (RGB565)
#define ST7789_COLOR_BLACK      0x0000
#define ST7789_COLOR_WHITE      0xFFFF
#define ST7789_COLOR_RED        0xF800
#define ST7789_COLOR_GREEN      0x07E0
#define ST7789_COLOR_BLUE       0x001F
#define ST7789_COLOR_YELLOW     0xFFE0
#define ST7789_COLOR_CYAN       0x07FF
#define ST7789_COLOR_MAGENTA    0xF81F
#define ST7789_COLOR_GRAY       0x8410
#define ST7789_COLOR_ORANGE     0xFD20

// ─────────────────────────────────────────────
// Data Structures
// ─────────────────────────────────────────────

/**
 * @brief Configuration structure for st7789_init()
 */
typedef struct {
    spi_host_device_t spi_host;     // SPI host (e.g. SPI3_HOST)
    gpio_num_t        cs_pin;       // Chip Select
    gpio_num_t        dc_pin;       // Data/Command
    gpio_num_t        rst_pin;      // Reset
    gpio_num_t        bl_pin;       // Backlight (Optional, GPIO_NUM_NC if unused)
    int               clk_speed_hz; // SPI Clock Speed (e.g. 40MHz)
} st7789_config_t;

// ─────────────────────────────────────────────
// Core APIs
// ─────────────────────────────────────────────

/**
 * @brief Initialize ST7789 display
 *
 * Configures GPIOs, registers SPI device, and runs initialization sequence.
 *
 * @param cfg         Configuration parameters
 * @return ESP_OK on success
 */
esp_err_t st7789_init(const st7789_config_t *cfg);

/**
 * @brief Set display backlight brightness
 * 
 * @param brightness_percent 0 to 100
 */
esp_err_t st7789_set_backlight(uint8_t brightness_percent);

/**
 * @brief Set the drawing window (Active area)
 */
esp_err_t st7789_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * @brief Set screen rotation
 * @param rotation 0-3 (0: 0°, 1: 90°, 2: 180°, 3: 270°)
 */
void st7789_set_rotation(uint8_t rotation);

// ─────────────────────────────────────────────
// High-Level Data Transfer
// ─────────────────────────────────────────────

/**
 * @brief Push a block of colors to the current window (Blocking)
 */
esp_err_t st7789_push_colors(const uint16_t *colors, uint32_t len);

/**
 * @brief Push a block of colors using DMA (Non-blocking)
 */
esp_err_t st7789_push_colors_dma(const uint16_t *colors, uint32_t len);

// ─────────────────────────────────────────────
// Geometric Drawing
// ─────────────────────────────────────────────

/**
 * @brief Fill the entire screen with a single color
 */
esp_err_t st7789_fill_screen(uint16_t color);

/**
 * @brief Draw a solid rectangle
 */
esp_err_t st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Draw a rectangle outline
 */
esp_err_t st7789_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Draw a single pixel
 */
esp_err_t st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Draw a line
 */
esp_err_t st7789_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/**
 * @brief Draw fast horizontal line
 */
void st7789_draw_fast_hline(uint16_t x, uint16_t y, uint16_t w, uint16_t color);

/**
 * @brief Draw fast vertical line
 */
void st7789_draw_fast_vline(uint16_t x, uint16_t y, uint16_t h, uint16_t color);

/**
 * @brief Draw circle outline
 */
void st7789_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

/**
 * @brief Fill a circle
 */
void st7789_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

// ─────────────────────────────────────────────
// Bitmap & Image Rendering
// ─────────────────────────────────────────────

/**
 * @brief Draw a 1-bit monochrome bitmap (e.g. for icons)
 * 
 * @param bitmap  Monochrome pixel data (1 bit per pixel)
 * @param color   Color for '1' bits
 * @param bg      Color for '0' bits
 */
void st7789_draw_bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *bitmap, uint16_t color, uint16_t bg);

/**
 * @brief Draw a full 16-bit RGB565 image
 * 
 * @param data    RGB565 pixel data
 */
void st7789_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);

// ─────────────────────────────────────────────
// Font / String APIs
// ─────────────────────────────────────────────

/**
 * @brief Draw a single character
 */
void st7789_draw_char(uint16_t x, uint16_t y, char c, const st7789_font_t *font, uint16_t color, uint16_t bg);

/**
 * @brief Draw a string
 */
void st7789_draw_string(uint16_t x, uint16_t y, const char *str, const st7789_font_t *font, uint16_t color, uint16_t bg);
