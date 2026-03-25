/**
 * @file edge_telemetry.c
 * @brief CAN Telemetry Service implementation
 */

#include "edge_telemetry.h"
#include "bsp_can.h"
#include "protocol_types.h"
#include "system_registry.h"
#include "esmu_protocol.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

// ─────────────────────────────────────────────
// Private constants
// ─────────────────────────────────────────────
#define TELEMETRY_HEALTH_INTERVAL_MS    3000
#define TELEMETRY_HEARTBEAT_INTERVAL_MS 5000

// ─────────────────────────────────────────────
// Private variables
// ─────────────────────────────────────────────
static TaskHandle_t telemetryTaskHandle = NULL;

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

/**
 * @brief Pack and send elevator health status
 */
void edge_telemetry_send_health_now(void) {
    system_registry_t data;
    if (!system_registry_read(&data)) return;

    ele_health_t packet = {0};
    packet.vibration = (int16_t)(data.metrics.vibration * 100);
    packet.speed = data.metrics.speed;
    packet.motion_state = (uint8_t)data.metrics.state;

    bsp_can_send(CAN_ID_ELE_HEALTH, (uint8_t*)&packet, sizeof(packet));
}

/**
 * @brief Pack and send node heartbeat
 */
static void send_heartbeat_packet(void) {
    edge_heartbeat_t packet = {0};
    
    // Map system state to protocol state
    node_state_t sys_state = system_registry_get_state();
    edge_state_t protocol_state = EDGE_STATE_INIT;
    
    if (sys_state == NODE_STATE_MONITORING) protocol_state = EDGE_STATE_RUNNING;
    else if (sys_state == NODE_STATE_ERROR) protocol_state = EDGE_STATE_ERROR;
    
    // Hardware diagnostics
    edge_health_t health = EDGE_HEALTH_OK;
    if (!mpu6050_test_connection()) {
        health = EDGE_HEALTH_SENSOR_LOST;
    }

    packet.edge_health = (uint8_t)health;
    packet.edge_state = (uint8_t)protocol_state;
    packet.error_code = 0; // Placeholder for granular error codes
    packet.uptime_sec = xTaskGetTickCount() / configTICK_RATE_HZ;

    bsp_can_send(CAN_ID_EDGE_HEALTH, (uint8_t*)&packet, sizeof(packet));
}

/**
 * @brief Main telemetry task for periodic broadcasting
 */
static void telemetry_task(void *argument) {
    uint32_t last_health = 0;
    uint32_t last_heartbeat = 0;
    
    for (;;) {
        uint32_t now = xTaskGetTickCount();
        
        // 1. Health Status (Periodic)
        if (now - last_health >= pdMS_TO_TICKS(TELEMETRY_HEALTH_INTERVAL_MS)) {
            edge_telemetry_send_health_now();
            last_health = now;
        }
        
        // 2. Node Heartbeat (Periodic)
        if (now - last_heartbeat >= pdMS_TO_TICKS(TELEMETRY_HEARTBEAT_INTERVAL_MS)) {
            send_heartbeat_packet();
            last_heartbeat = now;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

bool edge_telemetry_init(void) {
    // bsp_can_init() is called in main.c, so we just confirm registry is OK
    return true;
}

bool edge_telemetry_start(void) {
    if (telemetryTaskHandle == NULL) {
        BaseType_t ret = xTaskCreate(telemetry_task, "TelemetryTask", 512, NULL, tskIDLE_PRIORITY + 2, &telemetryTaskHandle);
        return (ret == pdPASS);
    }
    return true; // Already started
}

void edge_telemetry_broadcast_emergency(fault_code_t fault, uint8_t severity, uint8_t motion_state, int16_t value, uint16_t timestamp) {
    ele_emergency_t packet = {0};
    packet.fault_code = (uint8_t)fault;
    packet.severity = severity;
    packet.motion_state = motion_state;
    packet.fault_value = value;
    packet.timestamp = timestamp;
    
    bsp_can_send(CAN_ID_ELE_EMERGENCY, (uint8_t*)&packet, sizeof(packet));
}
