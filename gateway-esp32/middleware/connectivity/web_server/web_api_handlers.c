#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"
#include "system_registry.h"

static const char *TAG = "WEB_API";

esp_err_t web_api_status_json_handler(httpd_req_t *req) {
    system_status_registry_t snapshot;
    system_registry_get_snapshot(&snapshot);

    cJSON *root = cJSON_CreateObject();
    
    // System Section
    cJSON_AddNumberToObject(root, "uptime", snapshot.uptime_sec);
    cJSON_AddNumberToObject(root, "state", snapshot.current_state);
    cJSON_AddStringToObject(root, "sub_status", snapshot.sub_status);
    cJSON_AddBoolToObject(root, "fault_active", snapshot.fault_active);

    // Connectivity Section
    cJSON_AddBoolToObject(root, "wifi_connected", snapshot.wifi_level > 0);
    cJSON_AddNumberToObject(root, "wifi_level", snapshot.wifi_level);
    cJSON_AddNumberToObject(root, "wifi_rssi", snapshot.wifi_rssi);
    cJSON_AddBoolToObject(root, "mqtt_connected", snapshot.mqtt_connected);
    cJSON_AddBoolToObject(root, "cell_connected", snapshot.cellular_connected);
    cJSON_AddNumberToObject(root, "cell_level", snapshot.cellular_level);
    cJSON_AddStringToObject(root, "cell_operator", snapshot.cellular_operator);

    // Motion Section
    cJSON_AddStringToObject(root, "motion_state", snapshot.motion_state);
    cJSON_AddStringToObject(root, "ele_health", snapshot.elevator_health);
    cJSON_AddNumberToObject(root, "vibration", (double)snapshot.scaled_vibration);
    cJSON_AddNumberToObject(root, "speed", snapshot.ele_speed);

    // Edge Node Section
    cJSON_AddBoolToObject(root, "edge_connected", snapshot.edge_node_connected);
    cJSON_AddBoolToObject(root, "edge_armed", snapshot.edge_armed);

    const char *json_str = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_str);

    // Free resources
    free((void*)json_str);
    cJSON_Delete(root);

    return ESP_OK;
}
