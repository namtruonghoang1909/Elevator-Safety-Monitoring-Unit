/**
 * @file heartbeat.c
 * @brief Visual Heartbeat Service Implementation for STM32 Edge
 */

#include "heartbeat.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

// ─────────────────────────────────────────────
// Private variables
// ─────────────────────────────────────────────
static TaskHandle_t heartbeatTaskHandle = NULL;

// ─────────────────────────────────────────────
// Private function prototypes
// ─────────────────────────────────────────────
static void heartbeat_task(void *argument);

// ─────────────────────────────────────────────
// Task Logic
// ─────────────────────────────────────────────

/**
 * @brief Main heartbeat task loop (1Hz toggle)
 */
static void heartbeat_task(void *argument) {
    TickType_t last_wake_time = xTaskGetTickCount();

    for(;;) {
        HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1000));
    }
}

// ─────────────────────────────────────────────
// Global functions
// ─────────────────────────────────────────────

bool heartbeat_start(void) {
    BaseType_t ret = xTaskCreate(heartbeat_task, "HeartbeatTask", 128, NULL, tskIDLE_PRIORITY + 1, &heartbeatTaskHandle);
    return (ret == pdPASS);
}
