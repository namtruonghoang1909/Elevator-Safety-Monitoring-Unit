/**
 * @file widgets.h
 * @brief Reusable UI components for ESMU Dashboard
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "st7789.h"

// ─────────────────────────────────────────────
// Colors
// ─────────────────────────────────────────────
#define UI_COLOR_BG          ST7789_COLOR_BLACK
#define UI_COLOR_FG          ST7789_COLOR_WHITE
#define UI_COLOR_ACCENT      ST7789_COLOR_CYAN
#define UI_COLOR_SUCCESS     ST7789_COLOR_GREEN
#define UI_COLOR_WARNING     ST7789_COLOR_YELLOW
#define UI_COLOR_DANGER      ST7789_COLOR_RED
#define UI_COLOR_GRAY_DARK   0x4208

// ─────────────────────────────────────────────
// Widget APIs
// ─────────────────────────────────────────────

/**
 * @brief Draw a signal strength bar (0-4 bars) - for Cellular
 */
void widget_draw_cellular_icon(uint16_t x, uint16_t y, uint8_t level, uint16_t color);

/**
 * @brief Draw a WiFi signal icon (arcs)
 */
void widget_draw_wifi_icon(uint16_t x, uint16_t y, uint8_t level, uint16_t color);

/**
 * @brief Draw a cloud icon for MQTT
 */
void widget_draw_cloud_icon(uint16_t x, uint16_t y, bool connected, uint16_t color);

/**
 * @brief Draw a warning sign (!)
 */
void widget_draw_warning_sign(uint16_t x, uint16_t y, uint16_t size, uint16_t color);

/**
 * @brief Draw a status indicator (dot with label)
 */
void widget_draw_status_dot(uint16_t x, uint16_t y, bool active, const char* label);

/**
 * @brief Draw a horizontal progress bar or metric bar
 */
void widget_draw_progress_bar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t percent, uint16_t color);

/**
 * @brief Draw a header/title bar
 */
void widget_draw_header(const char* title);
