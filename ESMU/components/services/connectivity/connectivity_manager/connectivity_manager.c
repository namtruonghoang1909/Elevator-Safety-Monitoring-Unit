#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "connectivity_manager.h"
#include "system_registry.h"

static const char *TAG = "CONN_MGR";

static connectivity_config_t s_config;
static connectivity_state_t s_state = CONNECTIVITY_IDLE;
static bool s_running = false;
static uint32_t s_wifi_stable_counter = 0;
#define WIFI_STABILITY_THRESHOLD_SEC 5

static int8_t rssi_to_bars(int8_t rssi) {
    if (rssi == 0) return 0;
    if (rssi >= -55) return 4;
    if (rssi >= -65) return 3;
    if (rssi >= -75) return 2;
    if (rssi >= -85) return 1;
    return 0;
}

/**
 * @brief Connectivity Orchestrator Task
 */
static void connectivity_manager_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Connectivity Orchestrator Task started.");
    bool mqtt_started = false;

    while (s_running) {
        bool wifi_connected = wifi_sta_is_connected();
        bool mqtt_connected = mqtt_manager_is_connected();
        int8_t rssi = 0;

        if (!wifi_connected) {
            // No WiFi - Stop MQTT if it was running
            if (mqtt_started) {
                ESP_LOGW(TAG, "WiFi lost. Stopping MQTT...");
                esp_err_t err = mqtt_manager_stop();
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to stop MQTT manager: %s", esp_err_to_name(err));
                }
                mqtt_started = false;
            }
            s_wifi_stable_counter = 0;
            s_state = CONNECTIVITY_IDLE;
        } else {
            // WiFi is connected
            if (!mqtt_started) {
                s_wifi_stable_counter++;
                s_state = CONNECTIVITY_RECOVERING;

                if (s_wifi_stable_counter >= WIFI_STABILITY_THRESHOLD_SEC) {
                    ESP_LOGI(TAG, "WiFi stable for %d seconds. Starting MQTT...", WIFI_STABILITY_THRESHOLD_SEC);
                    esp_err_t err = mqtt_manager_start();
                    if (err == ESP_OK) {
                        mqtt_started = true;
                    } else {
                        ESP_LOGE(TAG, "Failed to start MQTT manager: %s", esp_err_to_name(err));
                        s_wifi_stable_counter = 0; // retry later
                    }
                } else {
                    ESP_LOGI(TAG, "WiFi connected. Waiting for stability... (%lu/%d)", 
                             s_wifi_stable_counter, WIFI_STABILITY_THRESHOLD_SEC);
                }
            } else {
                // MQTT has been started, check its status
                if (mqtt_connected) {
                    s_state = CONNECTIVITY_FULL;
                } else {
                    s_state = CONNECTIVITY_RECOVERING;
                }
            }

            // Monitor RSSI periodically
            rssi = connectivity_manager_get_rssi();
            if (rssi < -90) {
                ESP_LOGW(TAG, "Extremely weak signal: %d dBm. Connectivity may be unstable.", rssi);
            }
        }

        // Update System Registry for UI
        system_registry_update_wifi(wifi_connected ? rssi_to_bars(rssi) : 0, mqtt_connected);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Cleanup if task exits
    if (mqtt_started) {
        mqtt_manager_stop();
    }
    vTaskDelete(NULL);
}

esp_err_t connectivity_manager_init(const connectivity_config_t *config)
{
    if (config == NULL) return ESP_ERR_INVALID_ARG;

    s_config = *config;

    // Initialize underlying components
    esp_err_t ret = wifi_sta_init(&s_config.wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize wifi_sta");
        return ret;
    }

    ret = mqtt_manager_init(&s_config.mqtt_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mqtt_manager");
        return ret;
    }

    ESP_LOGI(TAG, "Connectivity Manager initialized.");
    return ESP_OK;
}

esp_err_t connectivity_manager_start(void)
{
    if (s_running) return ESP_ERR_INVALID_STATE;

    s_running = true;
    BaseType_t ret = xTaskCreate(connectivity_manager_task, "conn_mgr_task", 4096, NULL, 5, NULL);
    if (ret != pdPASS) {
        s_running = false;
        ESP_LOGE(TAG, "Failed to create connectivity task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Connectivity Manager started.");
    return ESP_OK;
}

esp_err_t connectivity_manager_stop(void)
{
    s_running = false;
    ESP_LOGI(TAG, "Connectivity Manager stopping...");
    // The task will clean up and delete itself
    return ESP_OK;
}

bool connectivity_manager_is_ready(void)
{
    return (s_state == CONNECTIVITY_FULL);
}

int8_t connectivity_manager_get_rssi(void)
{
    int8_t rssi = 0;
    wifi_sta_get_rssi(&rssi);
    return rssi;
}

connectivity_state_t connectivity_manager_get_state(void)
{
    return s_state;
}
