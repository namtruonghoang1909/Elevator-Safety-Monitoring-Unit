#ifndef WIFI_STA_H
#define WIFI_STA_H

#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief WiFi Station configuration structure
 */
typedef struct {
    const char *ssid;       /**< WiFi SSID */
    const char *password;   /**< WiFi Password */
    bool auto_reconnect;    /**< Enable/disable auto-reconnect */
} wifi_sta_user_config_t;

/**
 * @brief Initialize WiFi station mode
 * 
 * @param config Pointer to configuration structure
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_sta_init(const wifi_sta_user_config_t *config);

/**
 * @brief Connect to the configured WiFi network
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_sta_connect(void);

/**
 * @brief Disconnect from the current WiFi network
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_sta_user_disconnect(void);

/**
 * @brief Check if WiFi is currently connected and has an IP address
 * 
 * @return true if connected, false otherwise
 */
bool wifi_sta_is_connected(void);

/**
 * @brief Get the current IP address as a string
 * 
 * @param ip_str Buffer to store the IP string
 * @param len Length of the buffer
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_sta_get_ip(char *ip_str, size_t len);

/**
 * @brief Get the current signal strength (RSSI)
 * 
 * @param rssi Pointer to store the RSSI value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_sta_get_rssi(int8_t *rssi);

/**
 * @brief Start a task that logs WiFi health every 5 seconds
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t wifi_sta_start_monitoring(void);

#endif // WIFI_STA_H
