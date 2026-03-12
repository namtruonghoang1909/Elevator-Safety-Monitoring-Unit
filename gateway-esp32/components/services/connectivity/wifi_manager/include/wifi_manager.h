/**
 * @file wifi_manager.h
 * @brief WiFi Manager service (STA + AP modes)
 */

#pragma once

#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief WiFi user configuration
 */
typedef struct {
    const char *ssid;
    const char *password;
    bool auto_reconnect;
} wifi_manager_config_t;

/**
 * @brief Initialize the WiFi manager
 * 
 * @param config Configuration for STA mode
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_init(const wifi_manager_config_t *config);

/**
 * @brief Start WiFi in Station (STA) mode
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_start_sta(void);

/**
 * @brief Start WiFi in Access Point (AP) mode for provisioning
 * 
 * @param ap_ssid SSID for the AP
 * @param ap_pass Password for the AP (NULL for open)
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_start_ap(const char *ap_ssid, const char *ap_pass);

/**
 * @brief Stop WiFi
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_stop(void);

/**
 * @brief Check if WiFi is connected in STA mode
 * 
 * @return true if connected
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Get the current IP address string
 * 
 * @param ip_str Buffer to store IP (min 16 chars)
 * @param len Length of buffer
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_get_ip(char *ip_str, size_t len);

/**
 * @brief Get current RSSI (signal strength)
 * 
 * @param rssi Pointer to store RSSI value
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_get_rssi(int8_t *rssi);

/**
 * @brief Start a background task to log WiFi health
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_start_monitoring(void);
