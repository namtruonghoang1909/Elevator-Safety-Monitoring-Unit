#include <unity.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "connectivity_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TEST_WIFI_SSID "Bakito Coffee L1"
#define TEST_WIFI_PASS "chucngonmieng"
#define TEST_MQTT_URI  "mqtt://broker.emqx.io"

void setUp(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
}

void tearDown(void) {
    connectivity_manager_stop();
    extern esp_err_t wifi_sta_user_disconnect(void);
    wifi_sta_user_disconnect();
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void test_connectivity_init(void) {
    connectivity_config_t config = {
        .wifi_config = {
            .ssid = TEST_WIFI_SSID,
            .password = TEST_WIFI_PASS,
            .auto_reconnect = true
        },
        .mqtt_config = {
            .broker_uri = TEST_MQTT_URI,
            .client_id = "TEST_CLIENT"
        }
    };

    TEST_ASSERT_EQUAL(ESP_OK, connectivity_manager_init(&config));
}

void test_connectivity_start_and_status(void) {
    connectivity_config_t config = {
        .wifi_config = {
            .ssid = TEST_WIFI_SSID,
            .password = TEST_WIFI_PASS,
            .auto_reconnect = true
        },
        .mqtt_config = {
            .broker_uri = TEST_MQTT_URI,
            .client_id = "TEST_CLIENT_START"
        }
    };

    connectivity_manager_init(&config);
    TEST_ASSERT_EQUAL(ESP_OK, connectivity_manager_start());
    
    // Check initial state (should be IDLE then transition soon after task starts)
    connectivity_state_t state = connectivity_manager_get_state();
    TEST_ASSERT_TRUE(state == CONNECTIVITY_IDLE || state == CONNECTIVITY_WIFI_ONLY);
    
    // Check RSSI when just started (might be -127 or 0 depending on driver state)
    int8_t rssi = connectivity_manager_get_rssi();
    TEST_ASSERT_TRUE(rssi <= 0); // Either -127 (not connected) or 0 (connecting)
}

void test_connectivity_invalid_config(void) {
    // NULL config
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, connectivity_manager_init(NULL));
    
    // NULL SSID
    connectivity_config_t config = {
        .wifi_config = { .ssid = NULL, .password = "pass" },
        .mqtt_config = { .broker_uri = "uri" }
    };
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, connectivity_manager_init(&config));
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_connectivity_init);
    RUN_TEST(test_connectivity_start_and_status);
    RUN_TEST(test_connectivity_invalid_config);
    UNITY_END();
}
