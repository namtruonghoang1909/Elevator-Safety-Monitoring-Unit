/**
 * @file protocol.h
 * @brief Main ESMU distributed protocol header
 */

#pragma once

#include "protocol_types.h"
#include "protocol_packets.h"

/**
 * @brief Standard CAN IDs for the ESMU system (Lower ID = Higher Priority)
 */
#define CAN_ID_ELE_EMERGENCY    0x010   /**< Critical fault/emergency alert */
#define CAN_ID_ELE_HEALTH       0x100   /**< Real-time motion/balance data (100ms) */
#define CAN_ID_EDGE_HEALTH      0x200   /**< Edge node health and heartbeat (1000ms) */
