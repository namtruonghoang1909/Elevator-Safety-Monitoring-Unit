/**
 * @file main.c
 * @brief ESMU Application Entry Point (Minimalist)
 */

#include "system.h"
#include "system_registry.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    // 1. Initialize entire system (Drivers, Connectivity, UI, Motion)
    system_start();

    // 2. Main Uptime Loop
    uint32_t uptime_sec = 0;
    while (1) {
        system_registry_update_uptime(uptime_sec++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
