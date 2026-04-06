/**
 * @file config_screen.h
 * @brief Web Configuration Mode UI Screen
 */

#pragma once

#include "system_registry.h"

/**
 * @brief Draw the full Web Config screen
 * @param snapshot Snapshot of the system state
 */
void config_screen_draw_full(const system_status_registry_t* snapshot);

/**
 * @brief Update dynamic elements of the Config screen (status text, uptime)
 * @param snapshot Snapshot of the system state
 */
void config_screen_update(const system_status_registry_t* snapshot);
