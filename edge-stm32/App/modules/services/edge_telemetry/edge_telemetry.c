/**
 * @file edge_telemetry.c
 * @brief CAN Telemetry Service implementation with Conditional Health support
 */

#include "edge_logger.h"
#include "edge_telemetry.h"
#include "bsp_can.h"
#include "protocol_types.h"
#include "system_registry.h"
#include "esmu_protocol.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

#define EMERGENCY_COOLDOWN_MS           5000

static TaskHandle_t telemetryTaskHandle = NULL;
static uint32_t last_emergency_tick = 0;

// ─────────────────────────────────────────────
// Send functions
// ─────────────────────────────────────────────
static void send_health_packet(void) {
    system_registry_t data;
    if (!system_registry_read(&data)) return;

    // We send health even if not monitoring (armed) so Gateway has latest metrics
    ele_health_t packet = {0};
    packet.vibration = (int16_t)(data.metrics.vibration * 100);
    packet.speed = data.metrics.speed;
    packet.motion_state = (uint8_t)data.metrics.state;
    packet.health_status = (uint8_t)data.metrics.health_status;

    bsp_can_send(CAN_ID_ELE_HEALTH, (uint8_t*)&packet, sizeof(packet));
}

static void send_heartbeat_packet(void) {
    system_registry_t reg;
    if (!system_registry_read(&reg)) return; 

    edge_heartbeat_t packet = {0};
    edge_state_t protocol_state = reg.is_monitoring_active ? EDGE_STATE_RUNNING : EDGE_STATE_IDLE;
    
    edge_health_t health = EDGE_HEALTH_OK;
    if (!mpu6050_test_connection()) health = EDGE_HEALTH_SENSOR_LOST;

    packet.edge_health = (uint8_t)health;
    packet.edge_state = (uint8_t)protocol_state;
    packet.health_status = (uint8_t)reg.metrics.health_status;
    packet.error_code = 0; 
    packet.uptime_sec = (uint32_t)(xTaskGetTickCount() / configTICK_RATE_HZ);

    bsp_can_send(CAN_ID_EDGE_HEALTH, (uint8_t*)&packet, sizeof(packet));
}

void edge_telemetry_broadcast_emergency(fault_code_t fault, uint8_t severity, uint8_t motion_state, int16_t value, uint16_t timestamp) {
    uint32_t now = xTaskGetTickCount();
    
    // Check cooldown: only send if first time (0) or if cooldown has passed
    if (last_emergency_tick != 0 && (now - last_emergency_tick < pdMS_TO_TICKS(EMERGENCY_COOLDOWN_MS))) {
        // Log that we are suppressing this telemetry to avoid flooding
        edge_logger_printf("T-THROTTLE: %d", (int)fault);
        return; 
    }

    last_emergency_tick = now;

    ele_emergency_t packet = {0};
    packet.fault_code = (uint8_t)fault;
    packet.severity = severity;
    packet.motion_state = motion_state;
    packet.fault_value = value;
    packet.timestamp = timestamp;
    
    edge_logger_printf("T-SEND: %d", (int)fault);
    bsp_can_send(CAN_ID_ELE_EMERGENCY, (uint8_t*)&packet, sizeof(packet));
}


// ─────────────────────────────────────────────
// Core task
// ─────────────────────────────────────────────
static void telemetry_task(void *argument) {
    uint32_t last_health = 0;
    uint32_t last_heartbeat = 0;
    
    for (;;) {
        uint32_t now = xTaskGetTickCount();
        if (now - last_health >= pdMS_TO_TICKS(CAN_INTERVAL_MS_ELE_HEALTH)) {
            send_health_packet();
            last_health = now;
        }
        if (now - last_heartbeat >= pdMS_TO_TICKS(CAN_INTERVAL_MS_EDGE_HEARTBEAT)) {
            send_heartbeat_packet();
            last_heartbeat = now;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

bool edge_telemetry_start(void) {
    if (telemetryTaskHandle == NULL) {
        BaseType_t ret = xTaskCreate(telemetry_task, "TelemetryTask", 512, NULL, tskIDLE_PRIORITY + 2, &telemetryTaskHandle);
        return (ret == pdPASS);
    }
    return true; 
}

