/**
 * @file watchdog_service.h
 * @brief System Watchdog Service for Edge Node
 */

#pragma once

#include <stdbool.h>

/**
 * @brief Start the system watchdog task
 * @return true if task created successfully
 */
bool watchdog_service_start(void);
