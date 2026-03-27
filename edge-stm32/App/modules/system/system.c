/**
 * @file system.c
 * @brief System Controller implementation with Manual Arming support
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
#include "main.h"

// ─────────────────────────────────────────────
// Private constants
// ─────────────────────────────────────────────
#define DEBOUNCE_DELAY_MS 50

// ─────────────────────────────────────────────
// Task Logic
// ─────────────────────────────────────────────

static void system_task(void *argument) {
    TickType_t last_wake_time = xTaskGetTickCount();
    bool last_button_state = true; // Pull-up: True is unpressed
    bool is_armed = false;
    bool services_started = false;

    // 1. Initializations (Hardware ready, but no tasks/broadcasts yet)
    edge_logger_print("ESMU READY");
    edge_logger_print("PRESS TO START");

    for(;;) {
        // 1. Button Polling & Debouncing (PA15 - MONITOR_ACTIVATE_BUTTON)
        bool current_button_state = (HAL_GPIO_ReadPin(MONITOR_ACTIVATE_BUTTON_GPIO_Port, MONITOR_ACTIVATE_BUTTON_Pin) == GPIO_PIN_SET);
        
        if (last_button_state == true && current_button_state == false) {
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
            if (HAL_GPIO_ReadPin(MONITOR_ACTIVATE_BUTTON_GPIO_Port, MONITOR_ACTIVATE_BUTTON_Pin) == GPIO_PIN_RESET) {
                
                if (!services_started) {
                    // FIRST PRESS: Perform calibration and launch background tasks
                    system_start();
                    services_started = true;
                    is_armed = true;
                } else {
                    // SUBSEQUENT PRESSES: Toggle operational state
                    is_armed = !is_armed;
                }
                
                system_registry_t reg;
                system_registry_read(&reg);
                reg.is_monitoring_active = is_armed;
                reg.state = is_armed ? NODE_STATE_MONITORING : NODE_STATE_STANDBY;
                system_registry_write(&reg);

                if (is_armed) {
                    edge_logger_print("SYSTEM ACTIVE");
                } else {
                    edge_logger_print("PAUSED - STANDBY");
                }
            }
        }
        last_button_state = current_button_state;

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(100));
    }
}

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

bool system_core_init(void) {
    if (!system_registry_init()) return false;
    heartbeat_start();
    
    edge_logger_init(0x3C);
    motion_monitor_init();

    BaseType_t ret = xTaskCreate(system_task, "SystemTask", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
    return (ret == pdPASS);
}

bool system_start(void) {
    edge_logger_print("CALIBRATING...");
    edge_logger_print("DO NOT MOVE!");
    
    motion_monitor_calibrate();
    
    edge_logger_print("CALIB DONE");
    vTaskDelay(pdMS_TO_TICKS(500));

    // Start background work
    bool ret = true;
    ret = edge_telemetry_start();
    if(ret != true){
        edge_logger_print("tele start failed!");
    }
    ret = motion_monitor_start();
        if(ret != true){
        edge_logger_print("mm start failed!");
    }
    ret = watchdog_service_start();
        if(ret != true){
        edge_logger_print("wd start failed!");
    }
                    
    return ret;
}
