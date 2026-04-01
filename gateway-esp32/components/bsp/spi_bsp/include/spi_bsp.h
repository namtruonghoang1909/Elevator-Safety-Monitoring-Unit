/**
 * @file spi_bsp.h
 * @brief SPI master bus & device management abstraction (Board Support Package)
 *
 * Built on ESP-IDF spi_master.h.
 * Supports DMA transfers for high-speed display data.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define MAX_SPI_BUS_NUM    2
#define MAX_SPI_DEVICES_NUM 5

/**
 * @brief SPI bus information structure
 */
typedef struct {
    spi_host_device_t host_id;
    bool              initialized;
} spi_bus_info_t;

/**
 * @brief SPI device information structure
 */
typedef struct {
    spi_device_handle_t dev_handle;
    uint8_t             id;
    const char         *name;
} spi_device_info_t;

// ─────────────────────────────────────────────
// Bus & Device Management
// ─────────────────────────────────────────────

/**
 * @brief Initialize an SPI master bus
 * 
 * @param host_id SPI host (SPI2_HOST or SPI3_HOST)
 * @param sclk_pin Clock GPIO
 * @param mosi_pin MOSI GPIO
 * @param miso_pin MISO GPIO (GPIO_NUM_NC if not used)
 * @param dma_chan DMA channel (SPI_DMA_CH_AUTO recommended)
 * @return esp_err_t 
 */
esp_err_t spi_bsp_bus_init(spi_host_device_t host_id, gpio_num_t sclk_pin, gpio_num_t mosi_pin, gpio_num_t miso_pin, spi_dma_chan_t dma_chan);

/**
 * @brief Add a device to a bus
 * 
 * @param dev_id_out Pointer to store the assigned device ID
 * @param host_id Bus host to add the device to
 * @param cs_pin Chip Select GPIO
 * @param clock_speed_hz SPI clock speed
 * @param mode SPI mode (0-3)
 * @param dev_name Optional name for the device
 * @return esp_err_t 
 */
esp_err_t spi_bsp_add_device(uint8_t *dev_id_out, spi_host_device_t host_id, gpio_num_t cs_pin, int clock_speed_hz, int mode, const char *dev_name);

/**
 * @brief Deinitialize a specific SPI bus
 */
esp_err_t spi_bsp_deinit_bus(spi_host_device_t host_id);

/**
 * @brief Remove a device from its bus
 */
esp_err_t spi_bsp_remove_device(uint8_t dev_id);

// ─────────────────────────────────────────────
// Data Transfer Operations
// ─────────────────────────────────────────────

/**
 * @brief Execute a blocking SPI transfer (Polling or Interrupt based)
 * 
 * @param dev_id Device ID assigned during add_device
 * @param data Data buffer to send
 * @param len Length of data in bytes
 * @return esp_err_t 
 */
esp_err_t spi_bsp_transfer(uint8_t dev_id, const uint8_t *data, size_t len);

/**
 * @brief Execute a blocking SPI transfer with both TX and RX
 * 
 * @param dev_id Device ID
 * @param tx_data TX buffer
 * @param rx_data RX buffer
 * @param len Length in bytes
 * @return esp_err_t 
 */
esp_err_t spi_bsp_transfer_full_duplex(uint8_t dev_id, const uint8_t *tx_data, uint8_t *rx_data, size_t len);

/**
 * @brief Queue a DMA transfer (Non-blocking)
 * 
 * @param dev_id Device ID
 * @param data Data buffer (must be DMA-capable if using DMA)
 * @param len Length in bytes
 * @return esp_err_t 
 */
esp_err_t spi_bsp_queue_transfer(uint8_t dev_id, const uint8_t *data, size_t len);

/**
 * @brief Wait for a queued DMA transfer to complete
 * 
 * @param dev_id Device ID
 * @param timeout_ms Timeout in milliseconds
 * @return esp_err_t 
 */
esp_err_t spi_bsp_wait_transfer_done(uint8_t dev_id, uint32_t timeout_ms);
