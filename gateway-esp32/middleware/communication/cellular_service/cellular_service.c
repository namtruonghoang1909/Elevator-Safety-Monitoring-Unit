/**
 * @file cellular_service.c
 * @brief Implementation of Cellular Background Service
 */

#include "cellular_service.h"
#include "sim_a7680c.h"
#include "system_registry.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "CELLULAR_SVC";

// Event bits for internal signaling
#define CELLULAR_START_BIT    (1 << 0)
#define CELLULAR_RECOVER_BIT  (1 << 1)

static EventGroupHandle_t s_cellular_event_group = NULL;
static cellular_service_state_t s_current_state = CELLULAR_STATE_IDLE;
static TaskHandle_t s_cellular_task_handle = NULL;

/**
 * @brief Map signal quality (0-31) to 0-4 bars
 */
static int8_t csq_to_bars(int8_t csq) {
    if (csq == 0 || csq == 99) return 0;
    if (csq < 7) return 1;
    if (csq < 14) return 2;
    if (csq < 21) return 3;
    return 4;
}

/**
 * @brief Background Task implementing the Cellular FSM
 */
static void cellular_task(void *pvParameters) {
    sim_a7680c_info_t info;
    uint32_t fail_count = 0;
    uint32_t last_check_ms = 0;

    ESP_LOGI(TAG, "Cellular service task started.");

    while (1) {
        switch (s_current_state) {
            case CELLULAR_STATE_IDLE:
                // Wait indefinitely for start signal
                xEventGroupWaitBits(s_cellular_event_group, CELLULAR_START_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
                s_current_state = CELLULAR_STATE_INITIALIZING;
                ESP_LOGI(TAG, "Transitioning to INITIALIZING...");
                break;

            case CELLULAR_STATE_INITIALIZING:
                system_registry_set_subtext("SIM: Initializing...");
                if (sim_a7680c_init() == ESP_OK) {
                    s_current_state = CELLULAR_STATE_SEARCHING;
                    fail_count = 0;
                    ESP_LOGI(TAG, "Init OK, searching for network...");
                } else {
                    fail_count++;
                    ESP_LOGW(TAG, "Init failed (attempt %lu)", fail_count);
                    if (fail_count >= 3) {
                        s_current_state = CELLULAR_STATE_RECOVERING;
                    } else {
                        vTaskDelay(pdMS_TO_TICKS(5000));
                    }
                }
                break;

            case CELLULAR_STATE_SEARCHING:
                system_registry_set_subtext("SIM: Searching...");
                if (sim_a7680c_get_sim_info(&info) == ESP_OK) {
                    if (info.is_registered) {
                        s_current_state = CELLULAR_STATE_READY;
                        system_registry_set_subtext("SIM: Registered");
                        ESP_LOGI(TAG, "Registered on %s", info.operator_name);
                    } else {
                        ESP_LOGD(TAG, "Still searching... (CSQ: %d)", info.signal_strength);
                        vTaskDelay(pdMS_TO_TICKS(5000));
                    }
                    // Update registry even if not registered (for CSQ/Bars)
                    system_registry_update_cellular(csq_to_bars(info.signal_strength), info.signal_strength, info.is_registered, info.operator_name);
                } else {
                    s_current_state = CELLULAR_STATE_RECOVERING;
                }
                break;

            case CELLULAR_STATE_READY:
                {
                    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    if (now - last_check_ms >= 10000) { // Polling every 10s
                        if (sim_a7680c_get_sim_info(&info) == ESP_OK) {
                            if (!info.is_registered) {
                                ESP_LOGW(TAG, "Network lost!");
                                s_current_state = CELLULAR_STATE_SEARCHING;
                            }
                            system_registry_update_cellular(csq_to_bars(info.signal_strength), info.signal_strength, info.is_registered, info.operator_name);
                        } else {
                            ESP_LOGE(TAG, "Driver communication failure");
                            s_current_state = CELLULAR_STATE_RECOVERING;
                        }
                        last_check_ms = now;
                    }
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;

            case CELLULAR_STATE_RECOVERING:
                system_registry_set_subtext("SIM: Recovering...");
                ESP_LOGW(TAG, "Entering Recovery Sequence...");
                sim_a7680c_hw_reset();
                s_current_state = CELLULAR_STATE_INITIALIZING;
                fail_count = 0;
                break;

            case CELLULAR_STATE_ERROR:
                system_registry_set_subtext("SIM: Critical Error");
                vTaskDelay(pdMS_TO_TICKS(10000));
                break;
        }

        // Check if external recovery was requested
        if (xEventGroupGetBits(s_cellular_event_group) & CELLULAR_RECOVER_BIT) {
            xEventGroupClearBits(s_cellular_event_group, CELLULAR_RECOVER_BIT);
            s_current_state = CELLULAR_STATE_RECOVERING;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

esp_err_t cellular_service_init(void) {
    if (s_cellular_event_group != NULL) return ESP_OK;

    s_cellular_event_group = xEventGroupCreate();
    if (s_cellular_event_group == NULL) return ESP_ERR_NO_MEM;

    BaseType_t ret = xTaskCreate(cellular_task, "cellular_svc", 4096, NULL, 5, &s_cellular_task_handle);
    if (ret != pdPASS) return ESP_FAIL;

    return ESP_OK;
}

esp_err_t cellular_service_start(void) {
    if (!s_cellular_event_group) return ESP_ERR_INVALID_STATE;
    xEventGroupSetBits(s_cellular_event_group, CELLULAR_START_BIT);
    return ESP_OK;
}

cellular_service_state_t cellular_service_get_state(void) {
    return s_current_state;
}

esp_err_t cellular_service_send_sms(const char* phone, const char* message) {
    if (s_current_state != CELLULAR_STATE_READY) return ESP_ERR_INVALID_STATE;
    return sim_a7680c_send_sms(phone, message);
}

esp_err_t cellular_service_make_call(const char* phone) {
    if (s_current_state != CELLULAR_STATE_READY) return ESP_ERR_INVALID_STATE;
    return sim_a7680c_make_call(phone);
}
