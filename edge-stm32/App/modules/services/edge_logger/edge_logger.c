/**
 * @file edge_logger.c
 * @brief OLED-based local monitor implementation
 */

#include "edge_logger.h"
#include "ssd1306.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MAX_LOG_LINES 8
#define CHARS_PER_LINE 21 // 128 / 6

static char s_log_buffer[MAX_LOG_LINES][CHARS_PER_LINE + 1];
static int s_line_count = 0;

static void edge_logger_refresh(void) {
    // Only refresh the lines that have changed or just re-write all
    // To minimize jitter, we don't clear the whole screen here
    for (int i = 0; i < s_line_count; i++) {
        // Clear only this line's page first
        ssd1306_set_cursor(i, 0);
        uint8_t blank[SSD1306_WIDTH] = {0};
        ssd1306_write_data(blank, SSD1306_WIDTH);
        
        // Write the actual text
        ssd1306_write_string(s_log_buffer[i], i, 0);
    }
}

void edge_logger_init(uint8_t i2c_addr) {
    ssd1306_config_t cfg = {
        .address = i2c_addr,
        .contrast = 0xCF
    };
    ssd1306_init(&cfg);
    memset(s_log_buffer, 0, sizeof(s_log_buffer));
    s_line_count = 0;
    
    edge_logger_print("STM32 EDGE NODE");
    edge_logger_print("OLED MONITOR READY");
    edge_logger_print("--------------------");
}

void edge_logger_print(const char *msg) {
    int line_to_draw = 0;

    if (s_line_count >= MAX_LOG_LINES) {
        // Shift lines up
        for (int i = 0; i < MAX_LOG_LINES - 1; i++) {
            strncpy(s_log_buffer[i], s_log_buffer[i+1], CHARS_PER_LINE);
        }
        strncpy(s_log_buffer[MAX_LOG_LINES - 1], msg, CHARS_PER_LINE);
        line_to_draw = -1; // Flag to indicate full refresh needed due to scroll
    } else {
        strncpy(s_log_buffer[s_line_count], msg, CHARS_PER_LINE);
        line_to_draw = s_line_count;
        s_line_count++;
    }

    if (line_to_draw == -1) {
        edge_logger_refresh(); // Full refresh only on scroll
    } else {
        // Partial refresh: just draw the new line
        ssd1306_set_cursor(line_to_draw, 0);
        uint8_t blank[SSD1306_WIDTH] = {0};
        ssd1306_write_data(blank, SSD1306_WIDTH);
        ssd1306_write_string(s_log_buffer[line_to_draw], line_to_draw, 0);
    }
}

void edge_logger_printf(const char *fmt, ...) {
    char buf[CHARS_PER_LINE + 1];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    edge_logger_print(buf);
}
