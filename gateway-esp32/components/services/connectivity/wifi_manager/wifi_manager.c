#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "wifi_manager.h"

static const char *TAG = "WIFI_MGR";

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static wifi_manager_config_t s_config;
static bool s_is_connected = false;
static char s_ip_addr[16] = {0};
static bool s_wifi_initialized = false;
static esp_netif_t *s_sta_netif = NULL;
static esp_netif_t *s_ap_netif = NULL;

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
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

esp_err_t wifi_manager_init(const wifi_manager_config_t *config)
{
    if (config == NULL) return ESP_ERR_INVALID_ARG;
    s_config = *config;
    
    // Ensure auto-reconnect is enabled by default if not specified
    s_config.auto_reconnect = true;

    if (s_wifi_event_group == NULL) {
        s_wifi_event_group = xEventGroupCreate();
    }

    if (!s_wifi_initialized) {
        esp_err_t err = esp_netif_init();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;
        
        err = esp_event_loop_create_default();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

        s_sta_netif = esp_netif_create_default_wifi_sta();
        s_ap_netif = esp_netif_create_default_wifi_ap();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        err = esp_wifi_init(&cfg);
        if (err != ESP_OK) return err;

        err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL);
        if (err != ESP_OK) return err;

        err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL);
        if (err != ESP_OK) return err;
        
        s_wifi_initialized = true;
    }
    return ESP_OK;
}

esp_err_t wifi_manager_start_sta(void) {
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_OPEN,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    if (s_config.ssid) {
        strncpy((char *)wifi_config.sta.ssid, s_config.ssid, sizeof(wifi_config.sta.ssid) - 1);
        strncpy((char *)wifi_config.sta.password, s_config.password, sizeof(wifi_config.sta.password) - 1);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi Manager: STA mode started");
    return ESP_OK;
}

esp_err_t wifi_manager_start_ap(const char *ap_ssid, const char *ap_pass) {
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(ap_ssid),
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char *)wifi_config.ap.ssid, ap_ssid, sizeof(wifi_config.ap.ssid) - 1);
    if (ap_pass && strlen(ap_pass) > 0) {
        strncpy((char *)wifi_config.ap.password, ap_pass, sizeof(wifi_config.ap.password) - 1);
    } else {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi Manager: AP mode started (SSID: %s)", ap_ssid);
    return ESP_OK;
}

esp_err_t wifi_manager_stop(void) {
    return esp_wifi_stop();
}

bool wifi_manager_is_connected(void) {
    return s_is_connected;
}

esp_err_t wifi_manager_get_ip(char *ip_str, size_t len) {
    if (ip_str == NULL || len < 16) return ESP_ERR_INVALID_ARG;
    if (!s_is_connected) return ESP_ERR_INVALID_STATE;
    strncpy(ip_str, s_ip_addr, len - 1);
    return ESP_OK;
}

esp_err_t wifi_manager_get_rssi(int8_t *rssi) {
    if (rssi == NULL) return ESP_ERR_INVALID_ARG;
    wifi_ap_record_t ap_info;
    esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err == ESP_OK) *rssi = ap_info.rssi;
    return err;
}

static void wifi_monitoring_task(void *pvParameters) {
    while (1) {
        int8_t rssi = 0;
        bool connected = wifi_manager_is_connected();
        if (connected) wifi_manager_get_rssi(&rssi);
        
        ESP_LOGD(TAG, "WiFi Heartbeat - Connected: %d, RSSI: %d", connected, rssi);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

esp_err_t wifi_manager_start_monitoring(void) {
    BaseType_t ret = xTaskCreate(wifi_monitoring_task, "wifi_mon_task", 4096, NULL, 5, NULL);
    return (ret == pdPASS) ? ESP_OK : ESP_FAIL;
}
