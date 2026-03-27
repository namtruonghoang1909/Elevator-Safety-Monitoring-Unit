/**
 * @file motion_filters.c
 * @brief Implementation of signal processing filters
 */

#include "motion_filters.h"
#include <math.h>

float motion_filter_ema(float current, float previous, float alpha) {
    return (current * alpha) + (previous * (1.0f - alpha));
}

float motion_filter_deadband(float value, float threshold) {
    if (fabsf(value) < threshold) {
        return 0.0f;
    }
    return value;
}
