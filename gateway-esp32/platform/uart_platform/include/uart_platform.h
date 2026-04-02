/**
 * @file uart_platform.h
 * @brief Thread-safe Generic UART Platform Layer for ESP32
 * 
 * This component provides a robust interface for UART communication,
 * featuring a background task for data reception and thread-safe write access.
 */

#ifndef UART_PLATFORM_H
#define UART_PLATFORM_H

#include "esp_err.h"
#include "driver/uart.h"
#include <stdint.h>
#include <stdbool.h>

// ─────────────────────────────────────────────
// Macros & Constants
// ─────────────────────────────────────────────

#define UART_PLATFORM_DEFAULT_PORT      UART_NUM_2
#define UART_PLATFORM_DEFAULT_BAUD      115200
#define UART_PLATFORM_RX_BUF_SIZE       (2048)
#define UART_PLATFORM_TX_BUF_SIZE       (1024)

// ─────────────────────────────────────────────
// Type Definitions
// ─────────────────────────────────────────────

/**
 * @brief Callback function type for incoming data
 * @param data Received data buffer
 * @param len Length of received data
 */
typedef void (*platform_uart_data_cb_t)(const uint8_t* data, size_t len);

/**
 * @brief Configuration structure for UART Platform
 */
typedef struct {
    uart_port_t port;
    int baud_rate;
    int tx_io_num;
    int rx_io_num;
    platform_uart_data_cb_t data_cb; // Optional data callback
} platform_uart_config_t;

// ─────────────────────────────────────────────
// Global Function Declarations
// ─────────────────────────────────────────────

/**
 * @brief Initialize the UART Platform Layer
 * 
 * @param config Pointer to configuration structure
 * @return esp_err_t ESP_OK on success
 */
esp_err_t platform_uart_init(const platform_uart_config_t* config);

/**
 * @brief Deinitialize the UART Platform Layer
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t platform_uart_deinit(void);

/**
 * @brief Write data to the UART (Thread-safe)
 * 
 * @param data Pointer to data buffer
 * @param len Length of data to write
 * @return int Number of bytes written, or -1 on error
 */
int platform_uart_write(const uint8_t* data, size_t len);

/**
 * @brief Send a string (Thread-safe)
 * 
 * @param str The null-terminated string to send
 * @return esp_err_t ESP_OK on success
 */
esp_err_t platform_uart_send_str(const char* str);

/**
 * @brief Read data from the UART
 * 
 * This function reads directly from the UART driver buffer.
 * 
 * @param buf Buffer to store the data
 * @param max_len Maximum length of the buffer
 * @param timeout_ms Timeout in milliseconds
 * @return int Number of bytes read, or -1 on error
 */
int platform_uart_read(uint8_t* buf, size_t max_len, uint32_t timeout_ms);

/**
 * @brief Get the UART port number
 * 
 * @return uart_port_t The UART port number used by this Platform Layer
 */
uart_port_t platform_uart_get_port(void);

/**
 * @brief Set or update the data callback
 * 
 * @param data_cb The new callback function, or NULL to disable
 * @return esp_err_t ESP_OK on success
 */
esp_err_t platform_uart_set_data_cb(platform_uart_data_cb_t data_cb);

/**
 * @brief Flush the UART receive buffer
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t platform_uart_flush(void);

/**
 * @brief Get the number of bytes available in the RX buffer
 * 
 * @param out_len Pointer to store the number of bytes
 * @return esp_err_t ESP_OK on success
 */
esp_err_t platform_uart_get_available(size_t* out_len);

/**
 * @brief Read a line from the UART (terminated by \n or \r\n)
 * 
 * @param buf Buffer to store the line
 * @param max_len Maximum length of the buffer
 * @param timeout_ms Timeout in milliseconds
 * @return int Length of the line read (excluding terminator), or -1 on error/timeout
 */
int platform_uart_read_line(char* buf, size_t max_len, uint32_t timeout_ms);

/**
 * @brief Wait for data to be available
 * 
 * @param timeout_ms Timeout in milliseconds
 * @return esp_err_t ESP_OK if data is available, ESP_ERR_TIMEOUT otherwise
 */
esp_err_t platform_uart_wait_data(uint32_t timeout_ms);

#endif // UART_PLATFORM_H
