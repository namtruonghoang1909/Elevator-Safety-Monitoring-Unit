/**
 * @file edge_logger.h
 * @brief OLED-based local monitor for STM32 Edge Node
 */

#pragma once

#include <stdint.h>

/**
 * @brief Initialize the local OLED logger
 * @param i2c_addr The I2C address found by the scanner
 */
void edge_logger_init(uint8_t i2c_addr);

/**
 * @brief Print a message to the OLED (scrolling log)
 * @param msg String to display
 */
void edge_logger_print(const char *msg);

/**
 * @brief Formatted print to OLED
 */
void edge_logger_printf(const char *fmt, ...);
