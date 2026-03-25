/**
 * @file nvs_storage.h
 * @brief NVS Storage helper for persisting WiFi credentials and system config
 */

#pragma once

#include "esp_err.h"
#include <stddef.h>

/**
 * @brief Initialize the NVS flash storage
 * 
 * @return ESP_OK on success
 */
esp_err_t nvs_storage_init(void);

/**
 * @brief Save WiFi credentials to NVS
 * 
 * @param ssid WiFi SSID
 * @param password WiFi Password
 * @return ESP_OK on success
 */
esp_err_t nvs_storage_save_wifi_creds(const char *ssid, const char *password);

/**
 * @brief Load WiFi credentials from NVS
 * 
 * @param ssid Buffer to store SSID
 * @param ssid_len Length of SSID buffer
 * @param password Buffer to store password
 * @param password_len Length of password buffer
 * @return ESP_OK on success, ESP_ERR_NVS_NOT_FOUND if not present
 */
esp_err_t nvs_storage_load_wifi_creds(char *ssid, size_t ssid_len, char *password, size_t password_len);

/**
 * @brief Clear WiFi credentials from NVS
 * 
 * @return ESP_OK on success
 */
esp_err_t nvs_storage_clear_wifi_creds(void);
