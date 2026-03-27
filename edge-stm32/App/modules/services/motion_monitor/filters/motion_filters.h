/**
 * @file motion_filters.h
 * @brief Signal processing filters for Motion Monitor
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Apply Exponential Moving Average (EMA) filter
 */
float motion_filter_ema(float current, float previous, float alpha);

/**
 * @brief Apply a deadband (noise gate) to a value
 */
float motion_filter_deadband(float value, float threshold);
