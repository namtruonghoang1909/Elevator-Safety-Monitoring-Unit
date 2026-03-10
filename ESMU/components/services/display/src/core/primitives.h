/**
 * @file primitives.h
 * @brief Layer 1: Graphics Primitives for SSD1306
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

void display_primitives_clear(uint8_t *fb);
void display_draw_pixel(uint8_t *fb, int x, int y, bool color);
void display_draw_bitmap(uint8_t *fb, int x, int y, int w, int h, const uint8_t *data);
void display_draw_char(uint8_t *fb, int x, int y, char c);
void display_draw_string(uint8_t *fb, int x, int y, const char *str);
void display_draw_string_large(uint8_t *fb, int x, int y, const char *str);
void display_draw_hline(uint8_t *fb, int x1, int x2, int y);
void display_draw_ellipse(uint8_t *fb, int x0, int y0, int rx, int ry);
