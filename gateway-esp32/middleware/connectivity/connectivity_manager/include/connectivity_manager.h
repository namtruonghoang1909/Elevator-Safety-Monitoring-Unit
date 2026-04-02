/**
 * @file connectivity_manager.h
 * @brief Orchestrator for WiFi and MQTT connectivity
 *
 * Coordinates wifi_manager and mqtt_manager to ensure a stable network stack.
 * Handles stability timers, RSSI monitoring, and automatic reconnection logic.
 */

#ifndef CONNECTIVITY_MANAGER_H
#define CONNECTIVITY_MANAGER_H

#include <stdbool.h>
#include "esp_err.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"

/**
 * @brief Connectivity high-level states
 */
typedef enum {
    CONNECTIVITY_IDLE,          /**< Not initialized or not started */
    CONNECTIVITY_PROVISIONING,  /**< SoftAP Mode for WiFi setup */
    CONNECTIVITY_WIFI_ONLY,      /**< WiFi connected, MQTT not yet started or connected */
    CONNECTIVITY_FULL,          /**< Both WiFi and MQTT are connected */
    CONNECTIVITY_RECOVERING,    /**< WiFi is up, but MQTT is in a retry/backoff loop */
    CONNECTIVITY_ERROR          /**< Critical error state */
} connectivity_state_t;

/**
 * @brief Configuration for the Connectivity Manager
 */
typedef struct {
    wifi_manager_config_t wifi_config;   /**< WiFi configuration */
    mqtt_manager_config_t mqtt_config;   /**< MQTT configuration */
} connectivity_config_t;

/**
 * @brief Initialize the Connectivity Manager
 * 
 * Sets up internal WiFi and MQTT components.
 *
 * @param config Pointer to combined configuration
 * @return esp_err_t ESP_OK on success
 */
esp_err_t connectivity_manager_init(const connectivity_config_t *config);

/**
 * @brief Start the connectivity orchestrator
 * 
 * Begins the WiFi connection sequence.
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t connectivity_manager_start(void);

/**
 * @brief Stop all connectivity
 * 
 * Stops both MQTT and WiFi.
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t connectivity_manager_stop(void);

/**
 * @brief Check if the system is fully connected (WiFi + MQTT)
 * 
 * @return true if both WiFi and MQTT are connected, false otherwise
 */
bool connectivity_manager_is_ready(void);

/**
 * @brief Get the current WiFi signal strength
 * 
 * @return int8_t RSSI value in dBm
 */
int8_t connectivity_manager_get_rssi(void);

/**
 * @brief Get the current connectivity state
 * 
 * @return connectivity_state_t current state
 */
connectivity_state_t connectivity_manager_get_state(void);

#endif // CONNECTIVITY_MANAGER_H
