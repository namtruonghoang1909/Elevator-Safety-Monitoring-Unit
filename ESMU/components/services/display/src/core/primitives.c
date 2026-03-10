/**
 * @file primitives.c
 * @brief Layer 1 implementation
 */

#include "primitives.h"
#include "font.h" // Same folder
#include <string.h>

void display_primitives_clear(uint8_t *fb) {
    memset(fb, 0, 1024);
}

void display_draw_pixel(uint8_t *fb, int x, int y, bool color) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    int page = y / 8;
    int bit = y % 8;
    if (color) fb[page * 128 + x] |= (1 << bit);
    else fb[page * 128 + x] &= ~(1 << bit);
}

void display_draw_bitmap(uint8_t *fb, int x, int y, int w, int h, const uint8_t *data) {
    for (int col = 0; col < w; col++) {
        for (int bit = 0; bit < h; bit++) {
            bool color = (data[col] >> bit) & 0x01;
            display_draw_pixel(fb, x + col, y + bit, color);
        }
    }
}

void display_draw_char(uint8_t *fb, int x, int y, char c) {
    if (c < 32 || c > 126) c = '?';
    const uint8_t *bitmap = &FONT_5X7[(c - 32) * 5];
    display_draw_bitmap(fb, x, y, 5, 7, bitmap);
}

void display_draw_string(uint8_t *fb, int x, int y, const char *str) {
    while (*str) {
        display_draw_char(fb, x, y, *str++);
        x += 6; 
        if (x > 122) break;
    }
}

void display_draw_string_large(uint8_t *fb, int x, int y, const char *str) {
    while (*str) {
        char c = *str++;
        if (c < 32 || c > 126) c = '?';
        const uint8_t *bitmap = &FONT_5X7[(c - 32) * 5];
        
        // Scale 5x7 up to 10x14
        for (int col = 0; col < 5; col++) {
            for (int bit = 0; bit < 7; bit++) {
                if ((bitmap[col] >> bit) & 0x01) {
                    // Draw 2x2 block for each pixel
                    display_draw_pixel(fb, x + (col * 2),     y + (bit * 2),     true);
                    display_draw_pixel(fb, x + (col * 2) + 1, y + (bit * 2),     true);
                    display_draw_pixel(fb, x + (col * 2),     y + (bit * 2) + 1, true);
                    display_draw_pixel(fb, x + (col * 2) + 1, y + (bit * 2) + 1, true);
                }
            }
        }
        x += 12; // 10 width + 2 spacing
        if (x > 116) break;
    }
}

void display_draw_hline(uint8_t *fb, int x1, int x2, int y) {
    for (int x = x1; x <= x2; x++) display_draw_pixel(fb, x, y, true);
}

void display_draw_ellipse(uint8_t *fb, int x0, int y0, int rx, int ry) {
    int x, y;
    float d1, d2;
    x = 0;
    y = ry;

    // Initial decision parameter of region 1
    d1 = (ry * ry) - (rx * rx * ry) + (0.25 * rx * rx);

    while ((2 * ry * ry * x) <= (2 * rx * rx * y)) {
        display_draw_pixel(fb, x0 + x, y0 + y, true);
        display_draw_pixel(fb, x0 - x, y0 + y, true);
        display_draw_pixel(fb, x0 + x, y0 - y, true);
        display_draw_pixel(fb, x0 - x, y0 - y, true);

        if (d1 < 0) {
            x++;
            d1 = d1 + (2 * ry * ry * x) + (ry * ry);
        } else {
            x++;
            y--;
            d1 = d1 + (2 * ry * ry * x) - (2 * rx * rx * y) + (ry * ry);
        }
    }

    // Initial decision parameter of region 2
    d2 = ((ry * ry) * ((x + 0.5) * (x + 0.5))) + ((rx * rx) * ((y - 1) * (y - 1))) - (rx * rx * ry * ry);

    while (y >= 0) {
        display_draw_pixel(fb, x0 + x, y0 + y, true);
        display_draw_pixel(fb, x0 - x, y0 + y, true);
        display_draw_pixel(fb, x0 + x, y0 - y, true);
        display_draw_pixel(fb, x0 - x, y0 - y, true);

        if (d2 > 0) {
            y--;
            d2 = d2 + (rx * rx) - (2 * rx * rx * y);
        } else {
            y--;
            x++;
            d2 = d2 + (2 * ry * ry * x) - (2 * rx * rx * y) + (rx * rx);
        }
    }
}
