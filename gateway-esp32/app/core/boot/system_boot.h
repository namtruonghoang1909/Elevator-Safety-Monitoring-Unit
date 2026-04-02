/**
 * @file system_boot.h
 * @brief Service Boot Orchestrator for ESP32 Gateway
 */

#pragma once

#include "esp_err.h"

/**
 * @brief Begin the asynchronous system boot sequence
 * 
 * This starts a task that initializes all high-level services
 * in the correct order.
 * 
 * @return esp_err_t ESP_OK if task was created
 */
esp_err_t system_boot_begin(void);
