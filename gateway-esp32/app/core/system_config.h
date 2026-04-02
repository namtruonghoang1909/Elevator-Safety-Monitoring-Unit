#pragma once

#include "board_pins.h"

// Hardcoded for now - in production, these would be loaded from NVS
#define WIFI_SSID "Bakito Coffee L1"
#define WIFI_PASS "chucngonmieng"

/* CAN Hardware Configuration */
#define CAN_BAUD_RATE_KBPS   500

/* SIM7600/A7680C UART Configuration */
#define SIM_BAUD_RATE        9600

/* MQTT related */
#define BROKER_URI "mqtt://app.coreiot.io"
#define CLIENT_ID "ESMU_001"
#define CLIENT_USERNAME "esmu_001"
#define CLIENT_PASSWORD "esmu"

#define MQTT_TOPIC "v1/devices/me/telemetry"
