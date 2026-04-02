/**
 * @file motion_proxy.h
 * @brief Motion Proxy service for Distributed ESMU
 * 
 * This service acts as the Gateway's "Ear" for motion data received via CAN.
 * It decodes protocol packets and mirrors the state into the System Registry.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Initialize the motion proxy service
 * 
 * Starts the CAN receiver task and the watchdog.
 * @return ESP_OK on success
 */
esp_err_t motion_proxy_init(void);

/**
 * @brief Check if the remote motion data is fresh (Heartbeat Check)
 * 
 * @param timeout_ms Maximum allowed age of the data (default: 3000ms)
 * @return true if Edge node is online, false if offline
 */
bool motion_proxy_is_alive(uint32_t timeout_ms);
