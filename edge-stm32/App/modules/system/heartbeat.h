/**
 * @file heartbeat.h
 * @brief Visual Heartbeat Service for STM32 Edge
 */

#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <stdbool.h>

/**
 * @brief Initialize and start the heartbeat task
 * 
 * @return true if task was created successfully
 */
bool heartbeat_start(void);

#endif /* HEARTBEAT_H */
