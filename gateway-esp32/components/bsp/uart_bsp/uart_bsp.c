/**
 * @file uart_bsp.c
 * @brief Implementation of the generic thread-safe UART BSP.
 */

#include "uart_bsp.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>

static const char *TAG = "UART_BSP";

// ─────────────────────────────────────────────
// Private variables, macros
// ─────────────────────────────────────────────

static uart_port_t g_uart_port = UART_BSP_DEFAULT_PORT;
static SemaphoreHandle_t g_tx_mutex = NULL;
static SemaphoreHandle_t g_rx_mutex = NULL;
static uart_bsp_data_cb_t g_data_callback = NULL;
static TaskHandle_t g_uart_task_handle = NULL;
static bool g_is_initialized = false;

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

/**
 * @brief Background task to monitor UART and trigger callback
 */
static void uart_bsp_task(void *pvParameters) {
    uint8_t data[256];

    ESP_LOGI(TAG, "UART background task started");

    while (1) {
        // We only read if the callback is set AND we can get the RX mutex
        // This prevents the background task from "stealing" data from synchronous reads
        if (g_data_callback && xSemaphoreTake(g_rx_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            int len = uart_read_bytes(g_uart_port, data, sizeof(data), pdMS_TO_TICKS(10));
            if (len > 0) {
                g_data_callback(data, len);
            }
            xSemaphoreGive(g_rx_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ─────────────────────────────────────────────
// Global function definitions
// ─────────────────────────────────────────────

esp_err_t uart_bsp_init(const uart_bsp_config_t* config) {
    if (g_is_initialized) return ESP_OK;

    g_uart_port = config->port;
    g_data_callback = config->data_cb;

    uart_config_t uart_cfg = {
        .baud_rate = config->baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(g_uart_port, UART_BSP_RX_BUF_SIZE, UART_BSP_TX_BUF_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(g_uart_port, &uart_cfg));
    ESP_ERROR_CHECK(uart_set_pin(g_uart_port, config->tx_io_num, config->rx_io_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    g_tx_mutex = xSemaphoreCreateMutex();
    g_rx_mutex = xSemaphoreCreateMutex();

    if (g_data_callback) {
        xTaskCreate(uart_bsp_task, "uart_bsp_task", 4096, NULL, 10, &g_uart_task_handle);
    }

    g_is_initialized = true;
    ESP_LOGI(TAG, "Generic UART BSP initialized on port %d", g_uart_port);

    return ESP_OK;
}

esp_err_t uart_bsp_deinit(void) {
    if (!g_is_initialized) return ESP_OK;

    if (g_uart_task_handle) {
        vTaskDelete(g_uart_task_handle);
        g_uart_task_handle = NULL;
    }

    if (g_tx_mutex) {
        vSemaphoreDelete(g_tx_mutex);
        g_tx_mutex = NULL;
    }

    if (g_rx_mutex) {
        vSemaphoreDelete(g_rx_mutex);
        g_rx_mutex = NULL;
    }

    uart_driver_delete(g_uart_port);
    g_is_initialized = false;

    return ESP_OK;
}

int uart_bsp_write(const uint8_t* data, size_t len) {
    if (!g_is_initialized || !g_tx_mutex) return -1;

    xSemaphoreTake(g_tx_mutex, portMAX_DELAY);
    int written = uart_write_bytes(g_uart_port, (const char*)data, len);
    xSemaphoreGive(g_tx_mutex);

    return written;
}

esp_err_t uart_bsp_send_str(const char* str) {
    int len = strlen(str);
    if (uart_bsp_write((const uint8_t*)str, len) == len) {
        return ESP_OK;
    }
    return ESP_FAIL;
}

int uart_bsp_read(uint8_t* buf, size_t max_len, uint32_t timeout_ms) {
    if (!g_is_initialized || !g_rx_mutex) return -1;
    
    xSemaphoreTake(g_rx_mutex, portMAX_DELAY);
    int read = uart_read_bytes(g_uart_port, buf, max_len, pdMS_TO_TICKS(timeout_ms));
    xSemaphoreGive(g_rx_mutex);
    
    return read;
}

esp_err_t uart_bsp_set_data_cb(uart_bsp_data_cb_t data_cb) {
    if (!g_is_initialized) return ESP_FAIL;

    g_data_callback = data_cb;

    // Manage background task lifecycle
    if (g_data_callback && !g_uart_task_handle) {
        xTaskCreate(uart_bsp_task, "uart_bsp_task", 4096, NULL, 10, &g_uart_task_handle);
    } else if (!g_data_callback && g_uart_task_handle) {
        vTaskDelete(g_uart_task_handle);
        g_uart_task_handle = NULL;
    }

    return ESP_OK;
}

esp_err_t uart_bsp_flush(void) {
    if (!g_is_initialized) return ESP_FAIL;
    return uart_flush_input(g_uart_port);
}

esp_err_t uart_bsp_get_available(size_t* out_len) {
    if (!g_is_initialized) return ESP_FAIL;
    return uart_get_buffered_data_len(g_uart_port, out_len);
}

esp_err_t uart_bsp_wait_data(uint32_t timeout_ms) {
    if (!g_is_initialized) return ESP_FAIL;
    
    size_t available = 0;
    uint32_t elapsed = 0;
    while (elapsed < timeout_ms) {
        uart_get_buffered_data_len(g_uart_port, &available);
        if (available > 0) return ESP_OK;
        vTaskDelay(pdMS_TO_TICKS(10));
        elapsed += 10;
    }
    return ESP_ERR_TIMEOUT;
}

int uart_bsp_read_line(char* buf, size_t max_len, uint32_t timeout_ms) {
    if (!g_is_initialized || !g_rx_mutex || !buf || max_len < 2) return -1;

    xSemaphoreTake(g_rx_mutex, portMAX_DELAY);
    
    int total_read = 0;
    uint32_t start_tick = xTaskGetTickCount();
    
    while (total_read < (max_len - 1)) {
        uint8_t byte;
        int n = uart_read_bytes(g_uart_port, &byte, 1, pdMS_TO_TICKS(10));
        
        if (n > 0) {
            if (byte == '\n') {
                buf[total_read] = '\0';
                // Handle \r\n by removing \r if it was the previous char
                if (total_read > 0 && buf[total_read - 1] == '\r') {
                    buf[total_read - 1] = '\0';
                    total_read--;
                }
                xSemaphoreGive(g_rx_mutex);
                return total_read;
            }
            buf[total_read++] = (char)byte;
        }

        if ((xTaskGetTickCount() - start_tick) > pdMS_TO_TICKS(timeout_ms)) {
            break;
        }
    }
    
    buf[total_read] = '\0';
    xSemaphoreGive(g_rx_mutex);
    return (total_read > 0) ? total_read : -1;
}

uart_port_t uart_bsp_get_port(void) {
    return g_uart_port;
}
