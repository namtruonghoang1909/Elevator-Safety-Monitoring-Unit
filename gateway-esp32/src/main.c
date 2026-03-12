/**
 * @file main.c
 * @brief ESMU Application Entry Point
 */

#include "system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    // Initialize and start the entire system (Drivers, Connectivity, UI, Motion)
    system_start();

    // Keep the main task alive
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
