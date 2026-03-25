/**
 * @file protocol_packets.h
 * @brief Strictly packed 8-byte structs for CAN payloads (Improved Version)
 */

#pragma once

#include <stdint.h>
#include "protocol_types.h"

/* ============================================================
 * Elevator Health Status (CAN_ID_ELE_HEALTH)
 * Sent periodically (e.g., every 1–3 seconds)
 * ============================================================ */
typedef struct __attribute__((packed)) {
    int16_t vibration;        /**< Vibration magnitude (mg * 100) */
    int16_t speed;            /**< Elevator speed (mm/s or scaled) */
    uint8_t motion_state;     /**< motion_state_t (IDLE, UP, DOWN, STOP) */
} ele_health_t;


/* ============================================================
 * Elevator Emergency Fault (CAN_ID_ELE_EMERGENCY)
 * Sent immediately when fault is detected
 * ============================================================ */
typedef struct __attribute__((packed)) {
    uint8_t fault_code;       /**< fault_code_t */
    uint8_t severity;         /**< Severity level (1–5) */
    uint8_t motion_state;     /**< Context: motion_state_t */
    int16_t fault_value;      /**< Sensor value that triggered fault */
    uint16_t timestamp;       /**< Relative time (ms or tick count) */
} ele_emergency_t;


/* ============================================================
 * Edge Node Heartbeat (CAN_ID_EDGE_HEALTH)
 * Sent periodically (e.g., every 1 second)
 * ============================================================ */
typedef struct __attribute__((packed)) {
    uint8_t edge_health;      /**< edge_health_t (OK, WARN, ERROR) */
    uint8_t edge_state;       /**< edge_state_t (INIT, RUNNING, ERROR) */
    uint16_t error_code;      /**< Detailed error code (0 = no error) */
    uint32_t uptime_sec;      /**< Uptime since boot (seconds) */
} edge_heartbeat_t;


/* ============================================================
 * Test / Debug Packet (CAN_ID_TEST_SIGNAL)
 * Used for CAN validation, alignment, and packet loss detection
 * ============================================================ */
typedef struct __attribute__((packed)) {
    uint32_t uptime;          /**< Incrementing counter */
    uint8_t  seq;             /**< Sequence counter (0–255 rollover) */
    uint8_t  magic_byte;      /**< Must be 0xA5 for validation */
    uint8_t  reserved[2];     /**< Padding */
} test_packet_t;