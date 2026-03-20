/**
 * @file system.c
 * @brief System Controller implementation
 */

#include "system.h"
#include "system_registry.h"
#include "motion_monitor.h"
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
    TickType_t last_wake_time = xTaskGetTickCount();

    for(;;) {
        node_state_t current_state = system_registry_get_state();

        switch (current_state) {
            case NODE_STATE_INIT:
                // Wait for sensors to be ready (handled by defaultTask sequence)
                // For now, let's just wait 3 seconds and switch
                if (xTaskGetTickCount() > pdMS_TO_TICKS(3000)) {
                    system_registry_set_state(NODE_STATE_MONITORING);
                    edge_logger_print("SYSTEM: MONITORING");
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
    
    return true;
}

void system_start(void) {
    // Start Services
    if (!motion_monitor_init(NULL)) {
        edge_logger_print("ERR: MPU INIT FAIL");
    }
    motion_monitor_start();
    
    watchdog_service_start();

    // Start Controller
    xTaskCreate(system_task, "SystemTask", 512, NULL, tskIDLE_PRIORITY + 1, &systemTaskHandle);
}
