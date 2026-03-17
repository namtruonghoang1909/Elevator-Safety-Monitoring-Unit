/**
 * @file task.h
 * @brief Internal task and context for Motion Proxy
 */

#pragma once

#include "motion_proxy.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @brief Motion proxy internal context
 */
typedef struct {
    uint32_t last_heartbeat_ms;
    SemaphoreHandle_t lock;
} mp_task_ctx_t;

/**
 * @brief Initialize the proxy task and listener
 * @return ESP_OK on success
 */
esp_err_t mp_task_init(void);

/**
 * @brief Check heartbeat freshness
 */
bool mp_task_is_alive(uint32_t timeout_ms);
