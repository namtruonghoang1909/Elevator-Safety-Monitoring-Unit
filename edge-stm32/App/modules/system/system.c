/**
 * @file system.c
 * @brief System Controller implementation
 */

#include "system.h"
#include "heartbeat.h"
#include "system_registry.h"
#include "motion_monitor.h"
#include "edge_telemetry.h"
#include "watchdog_service.h"
#include "FreeRTOS.h"
#include "task.h"
#include "edge_logger.h"

// ─────────────────────────────────────────────
// Private function prototypes
// ─────────────────────────────────────────────
static void system_task(void *argument);

// ─────────────────────────────────────────────
// Private variables
// ─────────────────────────────────────────────

static TaskHandle_t systemTaskHandle = NULL;

// ─────────────────────────────────────────────
// Task Logic
// ─────────────────────────────────────────────

static void system_task(void *argument) {
    TickType_t task_start_tick = xTaskGetTickCount();
    TickType_t last_wake_time = xTaskGetTickCount();

    for(;;) {
        node_state_t current_state = system_registry_get_state();

        switch (current_state) {
            case NODE_STATE_INIT:
                // Wait for sensors to be ready (handled by defaultTask sequence)
                // We wait 5 seconds AFTER the system task has started (post-calibration)
                if (xTaskGetTickCount() - task_start_tick > pdMS_TO_TICKS(5000)) {
                    // Transition to MONITORING
                    system_registry_set_state(NODE_STATE_MONITORING);
                    edge_logger_print("SYSTEM: MONITORING");

                    // Start Background Services AFTER stability/calibration window
                    if (motion_monitor_start()) {
                        edge_logger_print("MONITOR STARTED");
                    } else {
                        edge_logger_print("ERR: MONITOR FAIL");
                    }

                    if (edge_telemetry_start()) {
                        edge_logger_print("TELEM STARTED");
                    } else {
                        edge_logger_print("ERR: TELEM FAIL");
                    }

                    if (watchdog_service_start()) {
                        edge_logger_print("WD STARTED");
                    } else {
                        edge_logger_print("ERR: WD FAIL");
                    }
                }
                break;

            case NODE_STATE_MONITORING:
                // Check if motion monitor has detected a fault
                system_registry_t data;
                if (system_registry_read(&data)) {
                    if (data.metrics.state == MOTION_STATE_SHAKING || 
                        data.metrics.state == MOTION_STATE_FREE_FALL) {
                        
                        system_registry_set_state(NODE_STATE_EMERGENCY);
                        edge_logger_print("!! EMERGENCY !!");
                    }
                }
                break;

            case NODE_STATE_EMERGENCY:
                // Latch state, wait for external reset command (via CAN)
                break;

            case NODE_STATE_ERROR:
                // Try recovery or wait for reset
                break;
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(100));
    }
}

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

bool system_core_init(void) {
    if (!system_registry_init()) return false;
    
    // Initialize OLED Logger
    edge_logger_init(0x3C);
    edge_logger_print("SYSTEM CORE INIT");
    
    // Start Visual Heartbeat
    heartbeat_start();
    
    BaseType_t ret = xTaskCreate(system_task, "SystemTask", 512, NULL, tskIDLE_PRIORITY + 1, &systemTaskHandle);
    if (ret != pdPASS) {
        edge_logger_print("ERR: SYS TASK FAIL");
    }
    
    return true;
}

bool system_start(void) {
    // 1. Initialize & Calibrate Sensors (Blocking)
    // motion_monitor_init calls motion_monitor_calibrate internally
    if (!motion_monitor_init(NULL)) {
        edge_logger_print("ERR: MPU INIT FAIL");
        return false;
    }
    
    edge_logger_print("SENSOR CALIBRATED");
    edge_logger_print("WAITING 5S...");

    return true;
}
