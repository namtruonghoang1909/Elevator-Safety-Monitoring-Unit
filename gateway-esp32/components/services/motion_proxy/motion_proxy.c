/**
 * @file motion_proxy.c
 * @brief Motion Proxy service implementation
 */

#include "motion_proxy.h"
#include "src/core/task.h"

esp_err_t motion_proxy_init(void) {
    return mp_task_init();
}

bool motion_proxy_is_alive(uint32_t timeout_ms) {
    return mp_task_is_alive(timeout_ms);
}
