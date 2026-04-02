#include <unity.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "connectivity_manager.h"
#include "system_config.h"
#include "esmu_protocol.h"
#include "display_service.h"
#include "system_registry.h"
#include "i2c_platform.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char *TAG = "TEST_MQTT_PROTO";

void setUp(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    // 1. Initialize I2C Bus (SSD1306 needs it)
    uint8_t bus_id = 0; // display_service expects bus 0
    TEST_ASSERT_EQUAL(ESP_OK, platform_i2c_bus_init(&bus_id, (gpio_num_t)I2C_SDA_PIN, (gpio_num_t)I2C_SCL_PIN));

    // 2. Initialize Status Registry and Display
    TEST_ASSERT_EQUAL(ESP_OK, system_registry_init());
    TEST_ASSERT_EQUAL(ESP_OK, display_service_init());
    
    // 3. Set state to TESTING to show the log view
    system_registry_set_state(SYSTEM_STATE_TESTING);
    display_service_log("MQTT TEST START");
    
    vTaskDelay(pdMS_TO_TICKS(500)); // Allow display task to refresh
}
void tearDown(void) {
    connectivity_manager_stop();
    display_service_log("TEARING DOWN...");
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void test_mqtt_protocol_packages(void) {
    display_service_log("CONNECTING...");
    connectivity_config_t config = {
        .wifi_config = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .auto_reconnect = true
        },
        .mqtt_config = {
            .broker_uri = BROKER_URI,
            .client_id = CLIENT_ID,
            .username = CLIENT_USERNAME,
            .password = CLIENT_PASSWORD,
            .port = 1883
        }
    };

    TEST_ASSERT_EQUAL(ESP_OK, connectivity_manager_init(&config));
    
    display_service_log("SSID:%s", config.wifi_config.ssid);
    display_service_log("PASS:%s", config.wifi_config.password);
    
    TEST_ASSERT_EQUAL(ESP_OK, connectivity_manager_start());

    // Wait for connection (30s timeout)
    bool connected = false;
    for (int i = 0; i < 30; i++) {
        if (connectivity_manager_is_ready()) {
            connected = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        display_service_log("WAITING... %ds", i);
    }
    TEST_ASSERT_TRUE_MESSAGE(connected, "Failed to connect to WiFi/MQTT within 30s");
    display_service_log("CONNECTED!");

    char payload[256];

    // 1. Test Package: Elevator Health
    display_service_log("PUB HEALTH");
    ele_health_t health = {
        .avg_tilt = 125,
        .max_tilt = 340,
        .balance = BALANCE_STATE_TILT_RIGHT,
        .health_score = 98
    };
    snprintf(payload, sizeof(payload), 
        "{\"type\":\"health\",\"avg_tilt\":%d,\"max_tilt\":%d,\"balance\":%d,\"score\":%d}",
        health.avg_tilt, health.max_tilt, health.balance, health.health_score);
    TEST_ASSERT_EQUAL(ESP_OK, mqtt_manager_publish(MQTT_TOPIC, payload, 0, false));

    // 2. Test Package: Edge Heartbeat
    display_service_log("PUB HEARTBEAT");
    edge_heartbeat_t hb = {
        .edge_health = EDGE_HEALTH_OK,
        .edge_state = EDGE_STATE_RUNNING,
        .uptime_sec = 1234
    };
    snprintf(payload, sizeof(payload),
        "{\"type\":\"heartbeat\",\"health\":%d,\"state\":%d,\"uptime\":%lu}",
        hb.edge_health, hb.edge_state, (unsigned long)hb.uptime_sec);
    TEST_ASSERT_EQUAL(ESP_OK, mqtt_manager_publish(MQTT_TOPIC, payload, 0, false));

    // 3. Test Package: Emergency/Fault
    display_service_log("PUB EMERGENCY");
    ele_emergency_t fault = {
        .fault_code = FAULT_SHAKE,
        .severity = 5,
        .fault_value = 2500
    };
    snprintf(payload, sizeof(payload),
        "{\"type\":\"emergency\",\"code\":%d,\"severity\":%d,\"value\":%d}",
        fault.fault_code, fault.severity, fault.fault_value);
    TEST_ASSERT_EQUAL(ESP_OK, mqtt_manager_publish(MQTT_TOPIC, payload, 1, true));

    display_service_log("SUCCESS!");
    // Wait a bit for all messages to be sent
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mqtt_protocol_packages);
    UNITY_END();
}
