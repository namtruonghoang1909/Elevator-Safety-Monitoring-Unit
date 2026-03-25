/**
 * @file system.h
 * @brief Redesigned System Orchestrator API
 */

#pragma once

#include "esp_err.h"

/**
 * @brief System-wide functional states
 */
typedef enum system_state_id_t {
    SYSTEM_STATE_IDLE,          /**< Power on, awaiting init */
    SYSTEM_STATE_INITIALIZING,  /**< Hardware discovery and self-test */
    SYSTEM_STATE_CONFIGURING,   /**< Web portal active for credentials */
    SYSTEM_STATE_MONITORING,    /**< Normal production operation */
    SYSTEM_STATE_TESTING,       /**< Running unit/integration tests */
    SYSTEM_STATE_ERROR          /**< Critical failure / Fail-Safe mode */
} system_state_id_t;

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
