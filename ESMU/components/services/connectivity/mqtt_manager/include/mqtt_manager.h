/**
 * @file mqtt_manager.h
 * @brief Modern MQTT client manager for ESP32 (wraps esp_mqtt_client)
 *
 * This component provides high-level APIs to initialize, start, and 
 * interact with an MQTT broker. It handles connection states and
 * provides thread-safe access to publishing and subscribing.
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Configuration structure for the MQTT Manager
 */
typedef struct {
    const char *broker_uri;   /**< Full broker URI (e.g., "mqtt://app.coreiot.io") */
    uint32_t port;            /**< Broker port (e.g., 1883 or 8883) */
    const char *client_id;    /**< Unique Client ID as registered in the broker */
    const char *username;     /**< MQTT Username */
    const char *password;     /**< MQTT Password */
    int keepalive;            /**< MQTT Keep-alive interval in seconds (default 60) */
    bool disable_auto_reconnect; /**< Set true to disable internal reconnection */
} mqtt_manager_config_t;

/**
 * @brief Initialize the MQTT Manager
 * 
 * Sets up the internal client handle and registers event handlers.
 *
 * @param config Pointer to the configuration struct
 * @return esp_err_t ESP_OK on success
 */
esp_err_t mqtt_manager_init(const mqtt_manager_config_t *config);

/**
 * @brief Start the MQTT client
 * 
 * Initiates the connection process to the broker.
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t mqtt_manager_start(void);

/**
 * @brief Stop the MQTT client
 * 
 * Disconnects and stops the client (cleanly closes the connection).
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t mqtt_manager_stop(void);

/**
 * @brief Publish data to a specific topic
 * 
 * @param topic The topic to publish to
 * @param data  The message payload (string)
 * @param qos   Quality of Service (0, 1, or 2)
 * @param retain Set true to retain the message on the broker
 * @return esp_err_t ESP_OK on success
 */
esp_err_t mqtt_manager_publish(const char *topic, const char *data, int qos, bool retain);

/**
 * @brief Subscribe to a specific topic
 * 
 * @param topic The topic to subscribe to
 * @param qos   Quality of Service (0, 1, or 2)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t mqtt_manager_subscribe(const char *topic, int qos);

/**
 * @brief Unsubscribe from a specific topic
 * 
 * @param topic The topic to unsubscribe from
 * @return esp_err_t ESP_OK on success
 */
esp_err_t mqtt_manager_unsubscribe(const char *topic);

/**
 * @brief Check if the client is currently connected to the broker
 * 
 * @return true if connected, false otherwise
 */
bool mqtt_manager_is_connected(void);

#endif // MQTT_MANAGER_H
