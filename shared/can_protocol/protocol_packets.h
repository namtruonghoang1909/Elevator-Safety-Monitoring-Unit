/**
 * @file protocol_packets.h
 * @brief Strictly packed 8-byte structs for CAN payloads
 */

#pragma once

#include <stdint.h>
#include "protocol_types.h"

/**
 * @brief Elevator health status message (CAN_ID_ELE_HEALTH)
 * 8 Bytes total
 */
typedef struct __attribute__((packed)) {
    int16_t avg_tilt;            /**< Average tilt magnitude (scaled * 100) */
    int16_t max_tilt;            /**< Peak tilt magnitude in window (scaled * 100) */
    uint8_t balance;             /**< balance_state_t (cast to uint8_t) */
    uint8_t health_score;        /**< 0-100 score representing overall stability */
    uint8_t reserved[2];         /**< Alignment padding */
} ele_health_t;

/**
 * @brief Elevator emergency fault message (CAN_ID_ELE_EMERGENCY)
 * 8 Bytes total
 */
typedef struct __attribute__((packed)) {
    uint8_t fault_code;          /**< fault_code_t (cast to uint8_t) */
    uint8_t severity;            /**< Severity level (1-5) */
    int16_t fault_value;         /**< Sensor value that triggered the fault (scaled) */
    uint8_t reserved[4];         /**< Alignment padding */
} ele_emergency_t;

/**
 * @brief Simple test packet for signal validation (CAN_ID_TEST_SIGNAL)
 * 8 Bytes total
 */
typedef struct __attribute__((packed)) {
    uint32_t uptime;             /**< Simple incrementing counter */
    uint8_t  magic_byte;         /**< Should be 0xA5 to confirm alignment */
    uint8_t  reserved[3];        /**< Padding */
} test_packet_t;

/**
 * @brief Edge node heartbeat and diagnostics (CAN_ID_EDGE_HEALTH)
 * 8 Bytes total
 */
typedef struct __attribute__((packed)) {
    uint8_t edge_health;         /**< edge_health_t (cast to uint8_t) */
    uint8_t edge_state;          /**< edge_state_t (INIT, RUNNING, ERROR) */
    uint32_t uptime_sec;         /**< Node uptime since boot */
    uint8_t  reserved[2];        /**< Alignment padding */
} edge_heartbeat_t;
