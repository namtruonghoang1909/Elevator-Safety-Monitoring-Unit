/**
 * @file config_screen.c
 * @brief Implementation of the Web Configuration Mode UI
 */

#include "config_screen.h"
#include "st7789.h"
#include "widgets.h"
#include <string.h>
#include <stdio.h>

// ─────────────────────────────────────────────
// Private variables
// ─────────────────────────────────────────────
static system_status_registry_t last_snap;

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

void config_screen_draw_full(const system_status_registry_t* snapshot) {
    // 1. Clear Screen
    st7789_fill_screen(UI_COLOR_BG);
    
    // 2. Draw Header
    widget_draw_header("WEB CONFIG MODE");
    
    // 3. Draw AP Instructions
    st7789_draw_string(20, 60, "1. Connect to WiFi:", &st7789_font_8x16, UI_COLOR_FG, UI_COLOR_BG);
    st7789_draw_string(40, 85, "SSID: ESMU-Setup", &st7789_font_8x16, UI_COLOR_ACCENT, UI_COLOR_BG);
    
    st7789_draw_string(20, 115, "2. Open Browser:", &st7789_font_8x16, UI_COLOR_FG, UI_COLOR_BG);
    st7789_draw_string(40, 140, "URL: 192.168.4.1", &st7789_font_8x16, UI_COLOR_ACCENT, UI_COLOR_BG);
    
    // 4. Status Area
    st7789_draw_string(20, 180, "STATUS:", &st7789_font_8x16, UI_COLOR_FG, UI_COLOR_BG);
    st7789_draw_string(20, 205, snapshot->sub_status, &st7789_font_8x16, UI_COLOR_WARNING, UI_COLOR_BG);
    
    // 5. Footer instruction
    st7789_draw_string(10, 260, "Hold D15 for 5s to exit", &st7789_font_8x16, UI_COLOR_GRAY_DARK, UI_COLOR_BG);
    
    memcpy(&last_snap, snapshot, sizeof(system_status_registry_t));
}

void config_screen_update(const system_status_registry_t* snapshot) {
    // Only update status if it changed
    if (strcmp(snapshot->sub_status, last_snap.sub_status) != 0) {
        // Clear status line (simple overwrite with spaces first or fill rect)
        // Here we just redraw it, st7789_draw_string with BG color will clear previous pixels
        st7789_draw_string(20, 205, "                    ", &st7789_font_8x16, UI_COLOR_BG, UI_COLOR_BG);
        st7789_draw_string(20, 205, snapshot->sub_status, &st7789_font_8x16, UI_COLOR_WARNING, UI_COLOR_BG);
        strncpy(last_snap.sub_status, snapshot->sub_status, sizeof(last_snap.sub_status)-1);
    }
}
