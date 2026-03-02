#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_sta.h"

static const char *TAG = "WIFI_STA";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static wifi_sta_user_config_t s_config;
static bool s_is_connected = false;
static char s_ip_addr[16] = {0};

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_is_connected = false;
        ESP_LOGI(TAG, "Disconnected from AP");
        if (s_config.auto_reconnect) {
            ESP_LOGI(TAG, "Retrying connection...");
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        esp_ip4addr_ntoa(&event->ip_info.ip, s_ip_addr, sizeof(s_ip_addr));
        ESP_LOGI(TAG, "Got IP: %s", s_ip_addr);
        s_is_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_sta_init(const wifi_sta_user_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    s_config = *config;
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    
    // Check if event loop is already created
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to create default event loop: %s", esp_err_to_name(err));
        return err;
    }

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    strncpy((char *)wifi_config.sta.ssid, s_config.ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, s_config.password, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_sta_init finished.");
    return ESP_OK;
}

esp_err_t wifi_sta_connect(void)
{
    return esp_wifi_connect();
}

esp_err_t wifi_sta_user_disconnect(void)
{
    return esp_wifi_disconnect();
}

bool wifi_sta_is_connected(void)
{
    return s_is_connected;
}

esp_err_t wifi_sta_get_ip(char *ip_str, size_t len)
{
    if (ip_str == NULL || len < 16) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_is_connected) {
        return ESP_ERR_INVALID_STATE;
    }
    strncpy(ip_str, s_ip_addr, len - 1);
    return ESP_OK;
}

esp_err_t wifi_sta_get_rssi(int8_t *rssi)
{
    if (rssi == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    wifi_ap_record_t ap_info;
    esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err == ESP_OK) {
        *rssi = ap_info.rssi;
    }
    return err;
}

static void wifi_monitoring_task(void *pvParameters)
{
    while (1) {
        int8_t rssi = 0;
        const char *strength = "Unknown";
        bool connected = wifi_sta_is_connected();
        
        if (connected) {
            if (wifi_sta_get_rssi(&rssi) == ESP_OK) {
                if (rssi >= -50) strength = "Strong";
                else if (rssi >= -70) strength = "Moderate";
                else if (rssi >= -85) strength = "Weak";
                else strength = "Very Weak";
            }
        } else {
            strength = "N/A";
        }

        printf("---- wifi health ----\n");
        printf("ssid: %s\n", s_config.ssid ? s_config.ssid : "Unknown");
        printf("state: %s\n", connected ? "connected" : "not connected");
        printf("strength: %s\n", strength);
        printf("---------------------\n");

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

esp_err_t wifi_sta_start_monitoring(void)
{
    BaseType_t ret = xTaskCreate(wifi_monitoring_task, "wifi_mon_task", 4096, NULL, 5, NULL);
    return (ret == pdPASS) ? ESP_OK : ESP_FAIL;
}
