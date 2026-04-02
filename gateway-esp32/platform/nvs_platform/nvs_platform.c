/**
 * @file nvs_platform.c
 * @brief NVS Storage helper for persisting WiFi credentials (Platform Layer)
 */

#include "nvs_platform.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "platform_nvs";
static const char *NAMESPACE = "esmu_storage";
static const char *KEY_SSID = "wifi_ssid";
static const char *KEY_PASS = "wifi_pass";

esp_err_t platform_nvs_init(void) {
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

esp_err_t platform_nvs_save_wifi_creds(const char *ssid, const char *password) {
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

cleanup:
    nvs_close(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving to NVS: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t platform_nvs_load_wifi_creds(char *ssid, size_t ssid_len, char *password, size_t password_len) {
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    err = nvs_get_str(handle, KEY_SSID, ssid, &ssid_len);
    if (err != ESP_OK) goto cleanup;

    err = nvs_get_str(handle, KEY_PASS, password, &password_len);

cleanup:
    nvs_close(handle);
    return err;
}

esp_err_t platform_nvs_clear_wifi_creds(void) {
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_erase_all(handle);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}
