#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"
#include "system_registry.h"
#include "ota_manager.h"

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

    // OTA Section
    cJSON_AddNumberToObject(root, "ota_progress", ota_manager_get_progress());

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

esp_err_t web_api_ota_post_handler(httpd_req_t *req) {
    char buf[256];
    int ret = httpd_req_recv(req, buf, req->content_len);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (root == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *url_item = cJSON_GetObjectItem(root, "url");
    if (!cJSON_IsString(url_item)) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing URL");
        return ESP_FAIL;
    }

    const char *url = url_item->valuestring;
    ESP_LOGI(TAG, "Requesting OTA from URL: %s", url);

    esp_err_t err = ota_manager_start(url);
    cJSON_Delete(root);

    if (err == ESP_OK) {
        httpd_resp_sendstr(req, "{\"status\":\"started\"}");
        return ESP_OK;
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to start OTA");
        return ESP_FAIL;
    }
}
