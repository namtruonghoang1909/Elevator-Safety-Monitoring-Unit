/**
 * @file motion_balance.h
 * @brief Elevator balance (tilt) detection logic
 */

#pragma once

#include "motion_monitor.h"

/**
 * @brief Evaluate accelerometer data to determine balance state
 * 
 * @param accel_x_g Filtered X acceleration
 * @param accel_y_g Filtered Y acceleration
 * @param threshold_g Gravity threshold (e.g., 0.1g)
 * @return balance_state_t Current tilt state
 */
balance_state_t mm_balance_update(float accel_x_g, float accel_y_g, float threshold_g);
