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

/**
 * @brief System-wide functional states
 */
typedef enum {
    SYSTEM_STATE_IDLE,          /**< Power on, awaiting init */
    SYSTEM_STATE_INITIALIZING,  /**< Hardware discovery and self-test */
    SYSTEM_STATE_CONFIGURING,   /**< Web portal active for credentials */
    SYSTEM_STATE_MONITORING,    /**< Normal production operation */
    SYSTEM_STATE_TESTING,       /**< Running unit/integration tests */
    SYSTEM_STATE_ERROR          /**< Critical failure / Fail-Safe mode */
} system_state_id_t;

/**
 * @brief Snapshot of the entire system state
 */
typedef struct {
    system_state_id_t current_state;
    int8_t wifi_level;         // 0-4 bars
    int8_t wifi_rssi;          // Raw RSSI in dBm
    bool mqtt_connected;
    bool cellular_connected;   // Cellular registration status
    int8_t cellular_level;     // 0-4 bars
    int8_t cellular_rssi;      // Raw CSQ (0-31)
    char cellular_operator[32]; // Current network provider
    bool edge_node_connected;  // Status of the remote STM32 node
    bool edge_armed;           // True if STM32 is actively monitoring
    char motion_state[16];     // "IDLE", "UP", "DOWN"
    char elevator_health[16];  // "GOOD", "SICK", "CRITICAL"
    bool has_wifi_creds;       // True if SSID/Pass are configured
    char sub_status[128];      // "Starting WiFi...", "Device configured!"
    uint32_t uptime_sec;
    bool fault_active;         // Triggers inverse video/alert view

    // --- Raw Protocol Data (For Telemetry) ---
    float scaled_vibration;    // New: Vibration in deg/s (scaled from raw)
    int16_t raw_vibration;
    int16_t ele_speed;         // New: Speed in mm/s
    uint8_t raw_health_status; // New: Raw health status (STABLE, WARN, EMERGENCY)
    uint8_t raw_motion_state;
    
    // Edge Node Status
    uint8_t raw_edge_health;
    uint8_t raw_edge_state;
    uint32_t raw_edge_uptime;
    uint16_t raw_edge_error;   // New: Edge error code

    // Fault History
    uint8_t last_fault_code;
    uint8_t last_fault_severity;
    int16_t last_fault_value;
    uint16_t last_fault_timestamp; // New: Timestamp of fault

    char emergency_phone[20];      // Target phone for SMS/Calls
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
 * @param rssi Signal strength in dBm
 * @param connected MQTT connection status
 */
void system_registry_update_wifi(int8_t level, int8_t rssi, bool connected);

/**
 * @brief Update Cellular connectivity info
 * @param level Signal strength (0-4 bars)
 * @param csq Raw CSQ (0-31)
 * @param connected Network registration status
 * @param operator Network provider name
 */
void system_registry_update_cellular(int8_t level, int8_t csq, bool connected, const char* operator);

/**
 * @brief Update Edge Node (Remote STM32) connection status
 * @param connected True if heartbeats are being received
 */
void system_registry_update_edge_status(bool connected);

/**
 * @brief Update motion and balance telemetry
 * @param motion Motion state string
 * @param balance Balance state string
 */
void system_registry_update_motion(const char* motion);

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
 * @brief Update the emergency phone number
 * @param phone Phone number string
 */
void system_registry_update_emergency_phone(const char* phone);

#include "esmu_protocol.h"

/**
 * @brief Update registry using CAN protocol health packet
 * @param pkt Pointer to received health packet
 */
void system_registry_update_from_protocol_health(const ele_health_t *pkt);

/**
 * @brief Update registry using CAN protocol heartbeat packet
 * @param pkt Pointer to received heartbeat packet
 */
void system_registry_update_from_protocol_heartbeat(const edge_heartbeat_t *pkt);

/**
 * @brief Update registry using CAN protocol emergency packet
 * @param pkt Pointer to received emergency packet
 */
void system_registry_update_from_protocol_emergency(const ele_emergency_t *pkt);

/**
 * @brief Get a thread-safe snapshot of the entire registry
 * @param out [out] Pointer to structure to receive the snapshot
 */
void system_registry_get_snapshot(system_status_registry_t *out);
