/**
 * @file ota_manager.h
 * @brief OTA Manager for ESMU Gateway (HTTPS based)
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include "esp_err.h"

// ─────────────────────────────────────────────
// Global function declarations
// ─────────────────────────────────────────────

/**
 * @brief Initialize the OTA manager.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ota_manager_init(void);

/**
 * @brief Start the OTA update process.
 * 
 * @param url The HTTPS URL of the firmware binary.
 * @return esp_err_t ESP_OK if the task was started successfully
 */
esp_err_t ota_manager_start(const char *url);

/**
 * @brief Get the current progress of the OTA update.
 * 
 * @return int Progress percentage (0-100), or -1 if no update in progress.
 */
int ota_manager_get_progress(void);

#endif // OTA_MANAGER_H
