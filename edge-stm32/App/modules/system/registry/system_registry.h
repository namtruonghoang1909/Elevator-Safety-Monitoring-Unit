/**
 * @file system_registry.h
 * @brief Thread-safe System Registry (Whiteboard) for Edge Node
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "motion_monitor.h"
#include "mpu6050.h"

// ─────────────────────────────────────────────
// Enums
// ─────────────────────────────────────────────

typedef enum {
    NODE_STATE_INIT = 0,
    NODE_STATE_STANDBY,
    NODE_STATE_MONITORING,
    NODE_STATE_EMERGENCY,
    NODE_STATE_ERROR
} node_state_t;

// ─────────────────────────────────────────────
// Data Structure
// ─────────────────────────────────────────────

typedef struct {
    motion_metrics_t metrics;    /**< Processed health data */
    mpu6050_raw_data_t raw;      /**< Real-time sensor data (using raw struct) */
    node_state_t state;          /**< Global FSM state */
    bool is_monitoring_active;   /**< Flag to enable/disable motion sensing & telemetry */
    uint32_t last_update_tick;   /**< For watchdog/freshness checks */
} system_registry_t;

// ─────────────────────────────────────────────
// APIs
// ─────────────────────────────────────────────

/**
 * @brief Initialize the registry and its mutex
 */
bool system_registry_init(void);

/**
 * @brief Thread-safe read from the registry
 */
bool system_registry_read(system_registry_t *out_data);

/**
 * @brief Thread-safe write to the registry
 */
bool system_registry_write(const system_registry_t *data);

/**
 * @brief Atomic state update
 */
void system_registry_set_state(node_state_t state);

/**
 * @brief Atomic state read
 */
node_state_t system_registry_get_state(void);
