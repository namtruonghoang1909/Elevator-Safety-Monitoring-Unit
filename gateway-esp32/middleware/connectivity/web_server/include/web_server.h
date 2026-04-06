/**
 * @file web_server.h
 * @brief HTTP Web Server for WiFi Provisioning
 */

#pragma once

#include "esp_err.h"
#include <stdbool.h>

/**
 * @brief Start the HTTP web server for provisioning
 * 
 * @return ESP_OK on success
 */
esp_err_t web_server_start(void);

/**
 * @brief Stop the HTTP web server
 * 
 * @return ESP_OK on success
 */
esp_err_t web_server_stop(void);

/**
 * @brief Check if new configuration has been received via the web portal
 * 
 * @param ssid_out Buffer to store received SSID
 * @param pass_out Buffer to store received Password
 * @param phone_out Buffer to store received Phone Number
 * @return true if configuration was received
 */
bool web_server_get_config(char *ssid_out, char *pass_out, char *phone_out);
