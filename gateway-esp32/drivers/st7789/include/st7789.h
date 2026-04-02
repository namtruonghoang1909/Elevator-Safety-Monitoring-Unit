/**
 * @file st7789.h
 * @brief ST7789 240x240 Color TFT LCD Driver
 * 
 * Built on top of spi_platform and pwm_platform.
 */

#pragma once

#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "spi_platform.h"

// ─────────────────────────────────────────────
// Constants
// ─────────────────────────────────────────────

#define ST7789_WIDTH  240
#define ST7789_HEIGHT 240

// Colors (RGB565)
#define ST7789_COLOR_BLACK   0x0000
#define ST7789_COLOR_WHITE   0xFFFF
#define ST7789_COLOR_RED     0xF800
#define ST7789_COLOR_GREEN   0x07E0
#define ST7789_COLOR_BLUE    0x001F
#define ST7789_COLOR_YELLOW  0xFFE0
#define ST7789_COLOR_CYAN    0x07FF
#define ST7789_COLOR_MAGENTA 0xF81F
#define ST7789_COLOR_GRAY    0x8410

// ─────────────────────────────────────────────
// Types
// ─────────────────────────────────────────────

typedef struct {
    spi_host_device_t spi_host;
    gpio_num_t        cs_pin;
    gpio_num_t        dc_pin;
    gpio_num_t        rst_pin;
    gpio_num_t        bl_pin;
    int               clk_speed_hz;
} st7789_config_t;

typedef struct {
    const uint8_t *data;
    uint8_t        width;
    uint8_t        height;
} st7789_font_t;

// ─────────────────────────────────────────────
// Fonts
// ─────────────────────────────────────────────
extern const st7789_font_t st7789_font_8x16;

// ─────────────────────────────────────────────
// Global APIs
// ─────────────────────────────────────────────

esp_err_t st7789_init(const st7789_config_t *cfg);
void      st7789_set_rotation(uint8_t m);
esp_err_t st7789_set_backlight(uint8_t brightness_percent);

// Graphics Primitives
esp_err_t st7789_fill_screen(uint16_t color);
esp_err_t st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
esp_err_t st7789_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
esp_err_t st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void      st7789_draw_fast_hline(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void      st7789_draw_fast_vline(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
esp_err_t st7789_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void      st7789_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void      st7789_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

// Text & Bitmaps
void      st7789_draw_char(uint16_t x, uint16_t y, char c, const st7789_font_t *font, uint16_t color, uint16_t bg);
void      st7789_draw_string(uint16_t x, uint16_t y, const char *str, const st7789_font_t *font, uint16_t color, uint16_t bg);
void      st7789_draw_bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *bitmap, uint16_t color, uint16_t bg);
void      st7789_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);

// Low-level Windowing
esp_err_t st7789_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
esp_err_t st7789_push_colors(const uint16_t *colors, uint32_t len);
esp_err_t st7789_push_colors_dma(const uint16_t *colors, uint32_t len);
