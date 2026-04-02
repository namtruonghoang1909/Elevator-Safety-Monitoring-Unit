/**
 * @file cellular_service.h
 * @brief Cellular Service for ESMU Gateway (Background Task)
 * 
 * Manages SIM module lifecycle, network registration, and provides
 * high-level APIs for SMS/Calls.
 */

#pragma once

#include "esp_err.h"
#include <stdbool.h>

/**
 * @brief Cellular Service States (FSM)
 */
typedef enum {
    CELLULAR_STATE_IDLE,          /**< Service not started */
    CELLULAR_STATE_INITIALIZING,  /**< Syncing and configuring module */
    CELLULAR_STATE_SEARCHING,     /**< Waiting for network registration */
    CELLULAR_STATE_READY,         /**< Registered and healthy */
    CELLULAR_STATE_RECOVERING,    /**< Handling transient errors */
    CELLULAR_STATE_ERROR          /**< Critical failure */
} cellular_service_state_t;

/**
 * @brief Initialize the cellular service
 * 
 * Creates the background task and prepares internal structures.
 * @return ESP_OK on success
 */
esp_err_t cellular_service_init(void);

/**
 * @brief Start the cellular service lifecycle
 * 
 * Moves state from IDLE to INITIALIZING.
 * @return ESP_OK on success
 */
esp_err_t cellular_service_start(void);

/**
 * @brief Get the current FSM state
 */
cellular_service_state_t cellular_service_get_state(void);

/**
 * @brief Send an SMS message
 * @param phone Recipient number
 * @param message Text content
 * @return ESP_OK if queued/sent
 */
esp_err_t cellular_service_send_sms(const char* phone, const char* message);

/**
 * @brief Initiate an emergency voice call
 * @param phone Recipient number
 * @return ESP_OK if initiated
 */
esp_err_t cellular_service_make_call(const char* phone);
