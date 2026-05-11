/**
 * @file ota_manager.c
 * @brief Implementation of HTTPS OTA Manager
 */

#include "ota_manager.h"
#include "esp_log.h"
#include "esp_https_ota.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

static const char *TAG = "OTA_MANAGER";

// ─────────────────────────────────────────────
// Private variables, macros
// ─────────────────────────────────────────────
static int _ota_progress = -1;
static TaskHandle_t _ota_task_handle = NULL;

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

/**
 * @brief Event handler for OTA events
 */
static void _ota_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (event_base == ESP_HTTPS_OTA_EVENT) {
        switch (event_id) {
            case ESP_HTTPS_OTA_START:
                ESP_LOGI(TAG, "OTA started");
                _ota_progress = 0;
                break;
            case ESP_HTTPS_OTA_CONNECTED:
                ESP_LOGI(TAG, "Connected to server");
                break;
            case ESP_HTTPS_OTA_GET_IMG_DESC:
                ESP_LOGI(TAG, "Reading image description");
                break;
            case ESP_HTTPS_OTA_VERIFY_CHIP_ID:
                ESP_LOGI(TAG, "Verifying chip ID");
                break;
            case ESP_HTTPS_OTA_DECRYPT_CB:
                ESP_LOGI(TAG, "Decrypting");
                break;
            case ESP_HTTPS_OTA_WRITE_FLASH:
                // Note: Actual progress calculation is done in the task loop
                break;
            case ESP_HTTPS_OTA_UPDATE_BOOT_PARTITION:
                ESP_LOGI(TAG, "Updating boot partition");
                break;
            case ESP_HTTPS_OTA_FINISH:
                ESP_LOGI(TAG, "OTA finished");
                _ota_progress = 100;
                break;
            case ESP_HTTPS_OTA_ABORT:
                ESP_LOGE(TAG, "OTA aborted");
                _ota_progress = -1;
                break;
        }
    }
}

/**
 * @brief The OTA update task
 */
static void _ota_task(void *pvParameter) {
    char *url = (char *)pvParameter;
    ESP_LOGI(TAG, "Starting OTA update from URL: %s", url);

    esp_http_client_config_t config = {
        .url = url,
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_begin failed: %s", esp_err_to_name(err));
        vPortFree(url);
        _ota_progress = -1;
        _ota_task_handle = NULL;
        vTaskDelete(NULL);
        return;
    }

    esp_app_desc_t app_desc;
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_get_img_desc failed: %s", esp_err_to_name(err));
        goto ota_end;
    }

    // Task loop to perform the update and track progress
    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        
        // Calculate progress
        int read_len = esp_https_ota_get_image_len_read(https_ota_handle);
        int total_len = esp_https_ota_get_image_size(https_ota_handle);
        if (total_len > 0) {
            _ota_progress = (read_len * 100) / total_len;
            if (_ota_progress % 10 == 0) {
                ESP_LOGI(TAG, "OTA Progress: %d%%", _ota_progress);
            }
        }
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle)) {
        err = esp_https_ota_finish(https_ota_handle);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "OTA upgrade successful! New firmware will load on next boot.");
            _ota_progress = 100; // Keep it at 100 to signify completion
            // Note: esp_restart() removed to allow manual reboot
        } else {
            ESP_LOGE(TAG, "esp_https_ota_finish failed: %s", esp_err_to_name(err));
            _ota_progress = -1;
        }
    }
 else {
        ESP_LOGE(TAG, "Complete data not received");
        err = ESP_FAIL;
    }

ota_end:
    esp_https_ota_abort(https_ota_handle);
    vPortFree(url);
    _ota_progress = -1;
    _ota_task_handle = NULL;
    vTaskDelete(NULL);
}

// ─────────────────────────────────────────────
// Global function definitions
// ─────────────────────────────────────────────

esp_err_t ota_manager_init(void) {
    // Register event handler if needed, though esp_https_ota uses internal events
    // We can register our own to spectate
    return esp_event_handler_register(ESP_HTTPS_OTA_EVENT, ESP_EVENT_ANY_ID, &_ota_event_handler, NULL);
}

esp_err_t ota_manager_start(const char *url) {
    if (_ota_task_handle != NULL) {
        ESP_LOGW(TAG, "OTA already in progress");
        return ESP_ERR_INVALID_STATE;
    }

    // Copy URL as the caller might free it
    char *url_copy = pvPortMalloc(strlen(url) + 1);
    if (url_copy == NULL) {
        return ESP_ERR_NO_MEM;
    }
    strcpy(url_copy, url);

    BaseType_t ret = xTaskCreate(_ota_task, "ota_task", 8192, url_copy, 5, &_ota_task_handle);
    if (ret != pdPASS) {
        vPortFree(url_copy);
        return ESP_FAIL;
    }

    return ESP_OK;
}

int ota_manager_get_progress(void) {
    return _ota_progress;
}
