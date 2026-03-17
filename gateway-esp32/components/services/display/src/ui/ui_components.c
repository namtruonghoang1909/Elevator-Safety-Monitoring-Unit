/**
 * @file ui_components.c
 * @brief Layer 2 implementation with Enhanced Monitoring and New States
 */

#include "ui_components.h"
#include "../core/primitives.h"
#include "../core/icons.h"
#include "system_registry.h"
#include <string.h>
#include <stdio.h>

static void ui_draw_elevator_box(uint8_t *fb, int x, int y, int w, int h, int tilt) {
    int offset = 0;
    if (tilt < 0) offset = -2;
    if (tilt > 0) offset = 2;

    for (int i = 0; i < w; i++) {
        display_draw_pixel(fb, x + i + offset, y, true);
        display_draw_pixel(fb, x + i - offset, y + h, true);
    }
    for (int i = 0; i <= h; i++) {
        float ratio = (float)i / h;
        int current_offset = offset - (int)(2.0f * offset * ratio);
        display_draw_pixel(fb, x + current_offset, y + i, true);
        display_draw_pixel(fb, x + w + current_offset, y + i, true);
    }
    int center_x = x + (w / 2);
    for (int i = 1; i <= 4; i++) display_draw_pixel(fb, center_x + offset, y - i, true);
    for (int i = 1; i <= 4; i++) display_draw_pixel(fb, center_x - offset, y + h + i, true);
}

void ui_draw_header(uint8_t *fb, int8_t wifi_level, bool mqtt_ok, bool edge_node_ok) {
    const uint8_t *wifi_icon;
    switch (wifi_level) {
        case 4: wifi_icon = WIFI_ICON_4_BARS; break;
        case 3: wifi_icon = WIFI_ICON_3_BARS; break;
        case 2: wifi_icon = WIFI_ICON_2_BARS; break;
        case 1: wifi_icon = WIFI_ICON_1_BARS; break;
        default: wifi_icon = WIFI_ICON_DISCONNECTED; break;
    }
    display_draw_bitmap(fb, 0, 0, 8, 8, wifi_icon);

    // MQTT Cloud Icon (16x8)
    const uint8_t *mqtt_icon = mqtt_ok ? ICON_CLOUD_ACTIVE : ICON_CLOUD_DISCONNECTED;
    display_draw_bitmap(fb, 12, 0, 16, 8, mqtt_icon);

    // Edge Node Status (Shifted right to account for 16px cloud)
    if (edge_node_ok) {
        display_draw_bitmap(fb, 32, 0, 8, 8, ICON_EDGE_OK);
    }

    // Center "ESMU" in the header (128/2 - (4*6)/2 = 64 - 12 = 52)
    display_draw_string(fb, 52, 0, "ESMU");

    display_draw_hline(fb, 0, 127, 9);
}

void ui_draw_footer(uint8_t *fb, uint32_t uptime_sec) {
    display_draw_hline(fb, 0, 127, 53);
    char buf[20];
    int d = uptime_sec / 86400;
    int h = (uptime_sec % 86400) / 3600;
    int m = (uptime_sec % 3600) / 60;
    snprintf(buf, sizeof(buf), "UP:%02dd %02dh %02dm", d, h, m);
    display_draw_string(fb, 0, 56, buf);

    if ((uptime_sec % 2) == 0) display_draw_bitmap(fb, 118, 55, 8, 8, ICON_HEART_V2);
}

void ui_draw_initializing_view(uint8_t *fb, const char *subtext) {
    display_draw_string_large(fb, 40, 25, "ESMU");
    display_draw_ellipse(fb, 64, 32, 35, 15);
    if (subtext) display_draw_string(fb, 10, 50, subtext);
}

void ui_draw_calibrating_view(uint8_t *fb, const char *subtext) {
    display_draw_string(fb, 25, 20, "CALIBRATING...");
    if (subtext) display_draw_string(fb, 15, 40, subtext);
}

void ui_draw_monitoring_view(uint8_t *fb, const char *motion, const char *balance, const char *health) {
    int tilt = 0;
    if (strstr(balance, "LEFT")) tilt = -1;
    else if (strstr(balance, "RIGHT")) tilt = 1;

    // Left side: Elevator visualization
    ui_draw_elevator_box(fb, 10, 16, 25, 30, tilt);
    
    // Draw movement arrows inside the box
    if (strcmp(motion, "UP") == 0) {
        display_draw_bitmap(fb, 18 + tilt, 27, 8, 8, ICON_ARROW_UP);
    } else if (strcmp(motion, "DOWN") == 0) {
        display_draw_bitmap(fb, 18 + tilt, 27, 8, 8, ICON_ARROW_DOWN);
    } else {
        display_draw_bitmap(fb, 18 + tilt, 27, 8, 8, ICON_IDLE_DOT);
    }

    // Right side: Status text
    display_draw_string(fb, 45, 16, "HEALTH:");
    display_draw_string(fb, 90, 16, health);

    display_draw_string(fb, 45, 30, "BAL:");
    display_draw_string(fb, 45, 40, balance);
}

void ui_draw_configuring_view(uint8_t *fb, const char *subtext) {
    display_draw_string(fb, 30, 12, "CONFIG MODE");
    display_draw_hline(fb, 25, 103, 22);

    if (subtext) {
        char buf[64];
        strncpy(buf, subtext, sizeof(buf)-1);
        buf[sizeof(buf)-1] = '\0';

        char *ssid = buf;
        char *ip = strchr(buf, '\n');
        
        if (ip) {
            *ip = '\0';
            ip++; // Move past the newline
            
            display_draw_string(fb, 5, 28, "Wi-Fi: ");
            display_draw_string(fb, 45, 28, ssid);
            
            display_draw_string(fb, 5, 40, "Web: ");
            display_draw_string(fb, 45, 40, ip);
        } else {
            display_draw_string(fb, 10, 32, subtext);
        }
    }
}

void ui_draw_error_view(uint8_t *fb, const char *msg) {
    display_draw_string(fb, 10, 20, "!!! EMERGENCY !!!");
    if (msg) display_draw_string(fb, 10, 35, msg);
}

void ui_draw_test_view(uint8_t *fb, const char *msg) {
    display_draw_string(fb, 0, 0, "--- TEST LOG ---");
    display_draw_hline(fb, 0, 127, 9);
    
    if (msg) {
        char buf[128];
        strncpy(buf, msg, sizeof(buf)-1);
        buf[sizeof(buf)-1] = '\0';
        
        char *line = strtok(buf, "\n");
        int y = 12;
        while (line != NULL && y < 53) {
            display_draw_string(fb, 0, y, line);
            y += 10;
            line = strtok(NULL, "\n");
        }
    }
}

void ui_draw_fail_safe_view(uint8_t *fb, const char *subtext) {
    display_draw_string_large(fb, 30, 20, "HALTED");
    if (subtext) display_draw_string(fb, 10, 45, subtext);
}
