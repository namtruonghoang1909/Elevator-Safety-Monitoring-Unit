/**
 * @file dashboard.c
 * @brief Implementation of the main status dashboard
 */

#include "dashboard.h"
#include "widgets.h"
#include <stdio.h>
#include <string.h>

// ─────────────────────────────────────────────
// Private Variables
// ─────────────────────────────────────────────
static system_status_registry_t last_snap = {0};
static bool last_emergency_overlay = false;

// ─────────────────────────────────────────────
// Private Helpers
// ─────────────────────────────────────────────

/**
 * @brief Formats seconds into human readable duration (e.g. 1d2h3m4s)
 */
static void format_uptime(uint32_t seconds, char *buf, size_t buf_len)
{
    uint32_t d = seconds / 86400;
    uint32_t h = (seconds % 86400) / 3600;
    uint32_t m = (seconds % 3600) / 60;
    uint32_t s = seconds % 60;

    int pos = 0;
    if (d > 0) pos += snprintf(buf + pos, buf_len - pos, "%lud", d);
    if (h > 0 || d > 0) pos += snprintf(buf + pos, buf_len - pos, "%luh", h);
    if (m > 0 || h > 0 || d > 0) pos += snprintf(buf + pos, buf_len - pos, "%lum", m);
    snprintf(buf + pos, buf_len - pos, "%lus", s);
}

static void draw_status_bar(const system_status_registry_t *snap)
{
    // Clear area below header (Header is now 36px)
    st7789_fill_rect(0, 36, ST7789_WIDTH, 24, UI_COLOR_BG);
    
    // WiFi Status
    st7789_draw_string(5, 40, "WiFi", &st7789_font_8x16, UI_COLOR_ACCENT, UI_COLOR_BG);
    widget_draw_wifi_icon(45, 40, snap->wifi_level, UI_COLOR_SUCCESS);
    
    // Cellular Status
    st7789_draw_string(110, 40, "Cell", &st7789_font_8x16, UI_COLOR_ACCENT, UI_COLOR_BG);
    widget_draw_cellular_icon(150, 40, snap->cellular_level, UI_COLOR_SUCCESS);

    // MQTT Status (Cloud Icon)
    widget_draw_cloud_icon(210, 40, snap->mqtt_connected, UI_COLOR_ACCENT);
}

static void draw_section_header(uint16_t y, const char* title)
{
    uint16_t len = strlen(title);
    uint16_t text_w = len * 8;
    uint16_t x = (ST7789_WIDTH - text_w) / 2;
    st7789_draw_string(x, y, title, &st7789_font_8x16, UI_COLOR_GRAY_DARK, UI_COLOR_BG);
}

static void draw_node_status(const system_status_registry_t *snap)
{
    draw_section_header(65, "--- NODE STATUS ---");
    
    widget_draw_status_dot(10, 85, true, "GATEWAY (ESP32)");
    widget_draw_status_dot(10, 105, snap->edge_node_connected, "EDGE    (STM32)");
}

static void draw_elevator_metrics(const system_status_registry_t *snap)
{
    // Clear the bottom area
    st7789_fill_rect(0, 130, ST7789_WIDTH, 110, UI_COLOR_BG);
    draw_section_header(130, "--- ELEVATOR METRICS ---");

    char buf[64];
    
    if (!snap->edge_armed) {
        st7789_draw_string(60, 165, "NOT ARMED", &st7789_font_8x16, UI_COLOR_WARNING, UI_COLOR_BG);
        st7789_draw_string(40, 185, "STANDBY MODE", &st7789_font_8x16, UI_COLOR_GRAY_DARK, UI_COLOR_BG);
    } else {
        // Vibration
        snprintf(buf, sizeof(buf), "VIB: %.2f deg/s", snap->scaled_vibration);
        st7789_draw_string(10, 150, buf, &st7789_font_8x16, UI_COLOR_FG, UI_COLOR_BG);
        
        // Motion & Health
        snprintf(buf, sizeof(buf), "MOVE: %s", snap->motion_state[0] ? snap->motion_state : "IDLE");
        st7789_draw_string(10, 175, buf, &st7789_font_8x16, UI_COLOR_FG, UI_COLOR_BG);

        uint16_t health_color = UI_COLOR_SUCCESS;
        if (snap->raw_health_status == 1) health_color = UI_COLOR_WARNING;
        else if (snap->raw_health_status == 2) health_color = UI_COLOR_DANGER;

        snprintf(buf, sizeof(buf), "HEALTH: %s", snap->elevator_health[0] ? snap->elevator_health : "OK");
        st7789_draw_string(10, 195, buf, &st7789_font_8x16, health_color, UI_COLOR_BG);
    }
    
    // Uptime (Human Readable)
    char time_buf[32];
    format_uptime(snap->uptime_sec, time_buf, sizeof(time_buf));
    snprintf(buf, sizeof(buf), "UPTIME: %s", time_buf);
    st7789_draw_string(10, 220, buf, &st7789_font_8x16, UI_COLOR_GRAY_DARK, UI_COLOR_BG);
}

static void draw_emergency_content(const system_status_registry_t *snap)
{
    // Replaces metrics area starting from Y=130
    st7789_fill_rect(0, 130, ST7789_WIDTH, 110, UI_COLOR_DANGER);
    
    // Smaller warning sign
    widget_draw_warning_sign(10, 145, 40, UI_COLOR_FG);
    
    st7789_draw_string(60, 145, "!! EMERGENCY !!", &st7789_font_8x16, UI_COLOR_FG, UI_COLOR_DANGER);
    
    char buf[32];
    sprintf(buf, "FAULT: %s", snap->elevator_health);
    st7789_draw_string(60, 170, buf, &st7789_font_8x16, UI_COLOR_FG, UI_COLOR_DANGER);
    
    sprintf(buf, "VALUE: %d", snap->last_fault_value);
    st7789_draw_string(60, 190, buf, &st7789_font_8x16, UI_COLOR_FG, UI_COLOR_DANGER);

    st7789_draw_string(10, 220, "CHECK SYSTEM IMMEDIATELY", &st7789_font_8x16, UI_COLOR_FG, UI_COLOR_DANGER);
}

// ─────────────────────────────────────────────
// Global APIs
// ─────────────────────────────────────────────

void dashboard_draw_full(const system_status_registry_t *snapshot, bool emergency_overlay)
{
    st7789_fill_screen(UI_COLOR_BG);
    widget_draw_header("ESMU");
    draw_status_bar(snapshot);
    draw_node_status(snapshot);
    
    if (emergency_overlay) {
        draw_emergency_content(snapshot);
    } else {
        draw_elevator_metrics(snapshot);
    }
    
    last_snap = *snapshot;
    last_emergency_overlay = emergency_overlay;
}

void dashboard_update(const system_status_registry_t *snapshot, bool emergency_overlay)
{
    // 1. Connectivity changes -> Update status bar
    if (snapshot->wifi_level != last_snap.wifi_level || 
        snapshot->cellular_level != last_snap.cellular_level ||
        snapshot->mqtt_connected != last_snap.mqtt_connected) 
    {
        draw_status_bar(snapshot);
    }

    // 2. Node connectivity changes -> Update node status
    if (snapshot->edge_node_connected != last_snap.edge_node_connected) {
        draw_node_status(snapshot);
    }

    // 3. Metrics or Overlay changes -> Update bottom area
    bool metrics_changed = (snapshot->edge_armed != last_snap.edge_armed) ||
                           (snapshot->raw_vibration != last_snap.raw_vibration) ||
                           (snapshot->raw_motion_state != last_snap.raw_motion_state) ||
                           (emergency_overlay != last_emergency_overlay);

    // Only redraw the full area on significant change or if armed state changed
    if (metrics_changed) {
        if (emergency_overlay) {
            draw_emergency_content(snapshot);
        } else {
            draw_elevator_metrics(snapshot);
        }
    } else if (!emergency_overlay) {
        // Surgical update for uptime only (every second)
        if (snapshot->uptime_sec != last_snap.uptime_sec) {
            char time_buf[32];
            char buf[64];
            format_uptime(snapshot->uptime_sec, time_buf, sizeof(time_buf));
            snprintf(buf, sizeof(buf), "UPTIME: %s", time_buf);
            // Clear only the uptime line
            st7789_fill_rect(0, 220, ST7789_WIDTH, 20, UI_COLOR_BG);
            st7789_draw_string(10, 220, buf, &st7789_font_8x16, UI_COLOR_GRAY_DARK, UI_COLOR_BG);
        }
    }

    last_snap = *snapshot;
    last_emergency_overlay = emergency_overlay;
}
