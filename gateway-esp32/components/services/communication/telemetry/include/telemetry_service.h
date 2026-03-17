/**
 * @file telemetry_service.h
 * @brief MQTT Telemetry Service for ESMU Gateway
 * 
 * Periodically publishes the System Registry state to the MQTT broker.
 */

#pragma once

#include "esp_err.h"
#include <stdint.h>

/**
 * @brief Initialize the telemetry service
 * 
 * Creates the background task for periodic MQTT publishing.
 * @return ESP_OK on success
 */
esp_err_t telemetry_service_init(void);

/**
 * @brief Trigger an immediate publish of the current state (e.g., on Fault)
 */
void telemetry_service_force_publish(void);
