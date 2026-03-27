/**
 * @file watchdog_service.c
 * @brief System Watchdog implementation
 */

#include "watchdog_service.h"
#include "system_registry.h"
#include "FreeRTOS.h"
#include "task.h"
#include "edge_logger.h"

// ─────────────────────────────────────────────
// Private function prototypes
// ─────────────────────────────────────────────
static void watchdog_task(void *argument);

// ─────────────────────────────────────────────
// Private variables
// ─────────────────────────────────────────────

static TaskHandle_t watchdogTaskHandle = NULL;

#define SENSOR_STALE_MS    1000  /**< If data is older than 1s, trigger Error */

// ─────────────────────────────────────────────
// Task Logic
// ─────────────────────────────────────────────

static void watchdog_task(void *argument) {
    TickType_t last_wake_time = xTaskGetTickCount();

    for(;;) {
        system_registry_t data;
        node_state_t current_state = system_registry_get_state();

        // 1. Check Data Freshness (Only if we are in Monitoring or Emergency)
        if (current_state == NODE_STATE_MONITORING || current_state == NODE_STATE_EMERGENCY) {
            if (system_registry_read(&data)) {
                uint32_t now = xTaskGetTickCount();
                
                // Detection: MPU6050 Task isn't updating the Registry
                if (now - data.last_update_tick > pdMS_TO_TICKS(SENSOR_STALE_MS)) {
                    system_registry_set_state(NODE_STATE_ERROR);
                    edge_logger_print("WATCHDOG: MPU STALE!");
                }
            }
        }

        // 2. (Future) Perform IWDG Reload (Independent HW Watchdog)
        // HAL_IWDG_Refresh(&hiwdg);

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(500)); // 2Hz is enough for health monitoring
    }
}

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

bool watchdog_service_start(void) {
    BaseType_t ret = xTaskCreate(watchdog_task, "WatchdogTask", 256, NULL, tskIDLE_PRIORITY + 4, &watchdogTaskHandle);
    return (ret == pdPASS);
}
