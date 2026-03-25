/**
 * @file system.h
 * @brief System Controller for Edge Node
 */

#pragma once

#include <stdbool.h>

/**
 * @brief Initialize all system-level resources (Mutexes, Registry, etc.)
 */
bool system_core_init(void);

/**
 * @brief Start all autonomous tasks (Motion, Display, System)
 * @return true if all services started successfully
 */
bool system_start(void);
