/**
 * @file esmu_protocol.h
 * @brief Unified ESMU Distributed Protocol Definition
 * 
 * This file acts as the main entry point for the protocol definition.
 * It includes the type and packet definitions and defines the CAN IDs.
 */

#pragma once

#include "protocol_packets.h"

// ─────────────────────────────────────────────
// CAN IDs (Lower ID = Higher Priority)
// ─────────────────────────────────────────────
#define CAN_ID_ELE_EMERGENCY    0x010   /**< Critical fault/emergency alert (Priority 1) */
#define CAN_ID_ELE_HEALTH       0x100   /**< Real-time motion/balance data (Priority 2) */
#define CAN_ID_EDGE_HEALTH      0x200   /**< Edge node health and heartbeat (Priority 3) */
#define CAN_ID_TEST_SIGNAL      0x7FF   /**< Physical line test signal (Lowest Priority) */

// ─────────────────────────────────────────────
// Communication Timing (ms)
// ─────────────────────────────────────────────
#define CAN_INTERVAL_MS_ELE_HEALTH       1000  /**< 1s reporting for motion metrics */
#define CAN_INTERVAL_MS_EDGE_HEARTBEAT   1000  /**< 1s reporting for node heartbeat */

// ─────────────────────────────────────────────
// Safety Timeouts (ms)
// ─────────────────────────────────────────────
/** Allow 5 missed heartbeats before declaring the edge node offline */
#define CAN_TIMEOUT_MS_EDGE_OFFLINE      5000
