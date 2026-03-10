/**
 * @file system_registry.h
 * @brief System Status Registry (The "Whiteboard")
 * 
 * Provides a thread-safe, centralized repository for system states and 
 * telemetry that can be updated by services and read by the UI.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "system.h" // For system_state_id_t

/**
 * @brief Snapshot of the entire system state
 */
typedef struct {
    system_state_id_t current_state;
    int8_t wifi_level;         // 0-4 bars
    bool mqtt_connected;
    char motion_state[16];     // "IDLE", "UP", "DOWN"
    char balance_state[16];    // "STABLE", "TILTING"
    char elevator_health[16];  // "GOOD", "SICK", "CRITICAL"
    bool has_wifi_creds; // True if SSID/Pass are configured
    char sub_status[32];       // "Starting WiFi...", "Device configured!"
    uint32_t uptime_sec;
    bool fault_active;         // Triggers inverse video/alert view
} system_status_registry_t;

/**
 * @brief Initialize the system registry and internal mutex
 * @return ESP_OK on success
 */
esp_err_t system_registry_init(void);

/**
 * @brief Update the global system state
 * @param state New system state (from system.h)
 */
void system_registry_set_state(system_state_id_t state);

/**
 * @brief Update sub-status text for booting or configuration phases
 * @param text Status message
 */
void system_registry_set_subtext(const char* text);

/**
 * @brief Update WiFi and MQTT connectivity info
 * @param level Signal strength (0-4 bars)
 * @param connected MQTT connection status
 */
void system_registry_update_wifi(int8_t level, bool connected);

/**
 * @brief Update motion and balance telemetry
 * @param motion Motion state string
 * @param balance Balance state string
 */
void system_registry_update_motion(const char* motion, const char* balance);

/**
 * @brief Update elevator health status
 * @param health Health string
 */
void system_registry_update_health(const char* health);

/**
 * @brief Mark if WiFi credentials have been set
 * @param set True if configured
 */
void system_registry_set_wifi_credentials(bool set);

/**
 * @brief Update the system uptime
 * @param uptime_sec Seconds since boot
 */
void system_registry_update_uptime(uint32_t uptime_sec);

/**
 * @brief Get a thread-safe snapshot of the entire registry
 * @param out [out] Pointer to structure to receive the snapshot
 */
void system_registry_get_snapshot(system_status_registry_t *out);
