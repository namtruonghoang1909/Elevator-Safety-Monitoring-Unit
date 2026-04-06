/**
 * @file widgets.c
 * @brief Implementation of reusable UI components
 */

#include "widgets.h"
#include <string.h>

// ─────────────────────────────────────────────
// Private Helpers
// ─────────────────────────────────────────────

/**
 * @brief Scaled character drawing helper
 */
static void draw_char_scaled(uint16_t x, uint16_t y, char c, const st7789_font_t *font, uint8_t scale, uint16_t color, uint16_t bg)
{
    if (c < 32 || c > 126) return;
    uint32_t offset = (c - 32) * font->height;
    const uint8_t *char_data = &font->data[offset];
    int byte_width = (font->width + 7) / 8;

    for (int j = 0; j < font->height; j++) {
        for (int i = 0; i < font->width; i++) {
            if (char_data[j * byte_width + i / 8] & (128 >> (i & 7))) {
                st7789_fill_rect(x + (i * scale), y + (j * scale), scale, scale, color);
            }
        }
    }
}

/**
 * @brief Scaled string drawing helper
 */
static void draw_string_scaled(uint16_t x, uint16_t y, const char *str, const st7789_font_t *font, uint8_t scale, uint16_t color, uint16_t bg)
{
    while (*str) {
        draw_char_scaled(x, y, *str, font, scale, color, bg);
        x += (font->width * scale);
        str++;
    }
}

// ─────────────────────────────────────────────
// Global APIs
// ─────────────────────────────────────────────

void widget_draw_cellular_icon(uint16_t x, uint16_t y, uint8_t level, uint16_t color)
{
    for (int i = 0; i < 4; i++) {
        uint16_t bar_h = (i + 1) * 4;
        uint16_t bar_color = (i < level) ? color : UI_COLOR_GRAY_DARK;
        st7789_fill_rect(x + (i * 5), y + (16 - bar_h), 3, bar_h, bar_color);
    }
}

void widget_draw_wifi_icon(uint16_t x, uint16_t y, uint8_t level, uint16_t color)
{
    uint16_t gray = UI_COLOR_GRAY_DARK;
    st7789_fill_circle(x + 8, y + 14, 2, (level >= 1) ? color : gray);
    uint16_t c2 = (level >= 2) ? color : gray;
    st7789_draw_line(x + 4, y + 10, x + 8, y + 7, c2);
    st7789_draw_line(x + 8, y + 7, x + 12, y + 10, c2);
    uint16_t c3 = (level >= 3) ? color : gray;
    st7789_draw_line(x + 2, y + 7, x + 8, y + 3, c3);
    st7789_draw_line(x + 8, y + 3, x + 14, y + 7, c3);
    uint16_t c4 = (level >= 4) ? color : gray;
    st7789_draw_line(x + 0, y + 4, x + 8, y + 0, c4);
    st7789_draw_line(x + 8, y + 0, x + 16, y + 4, c4);
}

void widget_draw_cloud_icon(uint16_t x, uint16_t y, bool connected, uint16_t color)
{
    uint16_t c = connected ? color : UI_COLOR_GRAY_DARK;
    st7789_fill_circle(x + 5, y + 10, 4, c);
    st7789_fill_circle(x + 10, y + 7, 5, c);
    st7789_fill_circle(x + 16, y + 10, 4, c);
    st7789_fill_rect(x + 5, y + 10, 11, 4, c);
    if (connected) {
        st7789_fill_circle(x + 10, y + 10, 2, ST7789_COLOR_WHITE);
    }
}

void widget_draw_warning_sign(uint16_t x, uint16_t y, uint16_t size, uint16_t color)
{
    // Draw Triangle
    for (int i = 0; i < size; i++) {
        st7789_draw_line(x + size/2 - i/2, y + i, x + size/2 + i/2, y + i, color);
    }
    // Draw Exclamation Point (!)
    uint16_t mid_x = x + size/2;
    st7789_fill_rect(mid_x - 1, y + size/4, 3, size/2, ST7789_COLOR_BLACK);
    st7789_fill_circle(mid_x, y + size*3/4 + 2, 2, ST7789_COLOR_BLACK);
}

void widget_draw_status_dot(uint16_t x, uint16_t y, bool active, const char* label)
{
    uint16_t dot_color = active ? UI_COLOR_SUCCESS : UI_COLOR_DANGER;
    st7789_fill_circle(x + 4, y + 8, 3, dot_color);
    st7789_draw_string(x + 12, y, label, &st7789_font_8x16, UI_COLOR_FG, UI_COLOR_BG);
}

void widget_draw_progress_bar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t percent, uint16_t color)
{
    if (percent > 100) percent = 100;
    st7789_draw_rect(x, y, w, h, UI_COLOR_GRAY_DARK);
    uint16_t fill_w = (w - 4) * percent / 100;
    if (fill_w > 0) {
        st7789_fill_rect(x + 2, y + 2, fill_w, h - 4, color);
    }
    if (fill_w < (w - 4)) {
        st7789_fill_rect(x + 2 + fill_w, y + 2, (w - 4) - fill_w, h - 4, UI_COLOR_BG);
    }
}

void widget_draw_header(const char* title)
{
    // Shorten to 36px height (3/4 of 48)
    st7789_fill_rect(0, 0, ST7789_WIDTH, 36, UI_COLOR_ACCENT);
    
    uint8_t scale = 2;
    uint16_t len = strlen(title);
    uint16_t text_w = len * (8 * scale);
    uint16_t x = (ST7789_WIDTH - text_w) / 2;
    uint16_t y = (36 - (16 * scale)) / 2;
    
    draw_string_scaled(x, y, title, &st7789_font_8x16, scale, UI_COLOR_BG, UI_COLOR_ACCENT);
}
