/**
 * @file st7789_fonts.h
 * @brief Font definitions for ST7789 driver
 */

#pragma once

#include <stdint.h>

typedef struct {
    const uint8_t *data;
    uint8_t width;
    uint8_t height;
} st7789_font_t;

// Standard 8x16 font (simplified ASCII 32-126)
extern const st7789_font_t st7789_font_8x16;
