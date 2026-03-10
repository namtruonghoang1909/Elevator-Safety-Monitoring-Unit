/**
 * @file ui_components.h
 * @brief Layer 2: Modular UI Elements for ESMU
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

void ui_draw_header(uint8_t *fb, int8_t wifi_level, bool mqtt_ok);
void ui_draw_footer(uint8_t *fb, uint32_t uptime_sec);
void ui_draw_initializing_view(uint8_t *fb, const char *subtext);
void ui_draw_calibrating_view(uint8_t *fb, const char *subtext);
void ui_draw_monitoring_view(uint8_t *fb, const char *motion, const char *balance, const char *health);
void ui_draw_configuring_view(uint8_t *fb, const char *subtext);
void ui_draw_error_view(uint8_t *fb, const char *msg);
void ui_draw_fail_safe_view(uint8_t *fb, const char *subtext);
