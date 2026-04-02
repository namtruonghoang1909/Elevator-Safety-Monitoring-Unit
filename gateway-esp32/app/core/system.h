/**
 * @file system.h
 * @brief Redesigned System Orchestrator API
 */

#pragma once

#include "esp_err.h"
#include "system_registry.h"

/**
 * @brief Stage 1: Memory & Queue Initialization
 * Must be called before any other system_report_event calls.
 */
esp_err_t system_core_init(void);

/**
 * @brief Stage 2: Orchestration Start
 * Spawns the system task and triggers the BOOT sequence.
 */
esp_err_t system_start(void);
