/**
 * @file edge_telemetry.h
 * @brief CAN Telemetry Service for ESMU Edge Node
 * 
 * This service is responsible for broadcasting real-time health and 
 * status information over the CAN bus.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "protocol_types.h"

// ─────────────────────────────────────────────
// Global APIs
// ─────────────────────────────────────────────

/**
 * @brief Initialize the telemetry service
 * @return true if initialization successful
 */
bool edge_telemetry_init(void);

/**
 * @brief Start the periodic telemetry task
 * @return true if task created successfully
 */
bool edge_telemetry_start(void);

/**
 * @brief Immediately send elevator health status packet
 */
void edge_telemetry_send_health_now(void);

/**
 * @brief Broadcast an immediate emergency message
 * 
 * @param fault Fault code to report
 * @param severity Severity level (1-5)
 * @param motion_state Current motion state context
 * @param value Triggering sensor value (scaled)
 * @param timestamp Relative timestamp (ticks)
 */
void edge_telemetry_broadcast_emergency(fault_code_t fault, uint8_t severity, uint8_t motion_state, int16_t value, uint16_t timestamp);
