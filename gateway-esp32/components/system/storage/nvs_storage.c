/**
 * @file nvs_storage.c
 * @brief NVS Storage helper for persisting WiFi credentials
 */

#include "nvs_storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "NVS_STORAGE";
static const char *NAMESPACE = "esmu_storage";
static const char *KEY_SSID = "wifi_ssid";
static const char *KEY_PASS = "wifi_pass";

esp_err_t nvs_storage_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to recover...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init NVS flash: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t nvs_storage_save_wifi_creds(const char *ssid, const char *password) {
    nvs_handle_t handle;
    esp_err_t err;

    if (ssid == NULL || password == NULL) return ESP_ERR_INVALID_ARG;

    err = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_str(handle, KEY_SSID, ssid);
    if (err != ESP_OK) goto cleanup;

    err = nvs_set_str(handle, KEY_PASS, password);
    if (err != ESP_OK) goto cleanup;

    err = nvs_commit(handle);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "WiFi credentials saved successfully");
    }

cleanup:
    nvs_close(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save WiFi creds: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t nvs_storage_load_wifi_creds(char *ssid, size_t ssid_len, char *password, size_t password_len) {
    nvs_handle_t handle;
    esp_err_t err;

    if (ssid == NULL || password == NULL) return ESP_ERR_INVALID_ARG;

    err = nvs_open(NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        if (err != ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        }
        return err;
    }

    err = nvs_get_str(handle, KEY_SSID, ssid, &ssid_len);
    if (err != ESP_OK) goto cleanup;

    err = nvs_get_str(handle, KEY_PASS, password, &password_len);

cleanup:
    nvs_close(handle);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Failed to load WiFi creds: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t nvs_storage_clear_wifi_creds(void) {
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_erase_key(handle, KEY_SSID);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) goto cleanup;

    err = nvs_erase_key(handle, KEY_PASS);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) goto cleanup;

    err = nvs_commit(handle);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "WiFi credentials cleared");
    }

cleanup:
    nvs_close(handle);
    return err;
}
