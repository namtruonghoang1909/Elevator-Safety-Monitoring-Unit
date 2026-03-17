/**
 * @file test_display_log.h
 * @brief Utility to use SSD1306 as a log during Unity tests
 */

#pragma once

#include "esp_err.h"

/**
 * @brief Initialize display for testing and set state to TESTING
 * 
 * This will initialize NVS, Registry, and Display Service if not already started.
 */
void test_display_log_init(void);

/**
 * @brief Add a line to the on-screen test log
 * 
 * Supports up to 5 lines (scrolls automatically).
 * Each line max ~20 chars.
 */
void test_display_log(const char *fmt, ...);

/**
 * @brief Clear the test log
 */
void test_display_log_clear(void);
