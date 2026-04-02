/**
 * @file sim_a7680c.h
 * @brief High-level SIM7600/A7680C/A7670C Driver for ESMU
 */

#ifndef SIM_A7680C_H
#define SIM_A7680C_H

#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief SIM Module & Subscriber Info
 */
typedef struct {
    char model[20];         /**< Module Model (e.g. A7680C) */
    char revision[32];      /**< Firmware Revision */
    char imei[20];          /**< Hardware Identity (Module) */
    char imsi[20];          /**< Subscriber Identity (SIM Card) */
    char phone_number[20];  /**< Mobile Number (If stored on SIM) */
    char operator_name[32]; /**< Current Network Provider */
    int signal_strength;    /**< 0-31 (CSQ) */
    bool is_registered;     /**< Network registration status (CREG or CEREG) */
    float voltage;          /**< Supply voltage (from AT+CBC) */
    int temp;               /**< Module temperature (from AT+CPMUTEMP) */
} sim_a7680c_info_t;

/**
 * @brief Initialize the SIM driver and module
 * Performs: Sync -> Echo Off -> Verbose Errors -> Radio On -> Identify
 */
esp_err_t sim_a7680c_init(void);

/**
 * @brief Retrieve all SIM and Module credentials
 */
esp_err_t sim_a7680c_get_sim_info(sim_a7680c_info_t* info);

/**
 * @brief Run deep diagnostics on SIM and Network status
 */
esp_err_t sim_a7680c_check_status(void);

/**
 * @brief Manually set the Module IMEI
 * NOTE: Use only if Module IMEI is blank/unknown.
 */
esp_err_t sim_a7680c_set_imei(const char* imei);

/**
 * @brief Perform a hardware reset via GPIO 5
 */
esp_err_t sim_a7680c_hw_reset(void);

/**
 * @brief Send an SMS message
 */
esp_err_t sim_a7680c_send_sms(const char* phone, const char* message);

/**
 * @brief Initiate a voice call
 */
esp_err_t sim_a7680c_make_call(const char* phone);

/**
 * @brief Terminate any active call
 */
esp_err_t sim_a7680c_hang_up(void);

/**
 * @brief Basic communication check
 */
esp_err_t sim_a7680c_ping(void);

#endif // SIM_A7680C_H
