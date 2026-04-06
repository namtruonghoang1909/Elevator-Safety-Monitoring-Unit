/**
 * @file dashboard.h
 * @brief Main status dashboard for ESMU Gateway
 */

#pragma once

#include "system_registry.h"

/**
 * @brief Full refresh of the dashboard
 * @param snapshot snapshot of system state
 * @param emergency_overlay true to show emergency instead of metrics
 */
void dashboard_draw_full(const system_status_registry_t *snapshot, bool emergency_overlay);

/**
 * @brief Partial/Differential update of the dashboard
 * @param snapshot new snapshot to compare and update
 * @param emergency_overlay true to show emergency instead of metrics
 */
void dashboard_update(const system_status_registry_t *snapshot, bool emergency_overlay);
