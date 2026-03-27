#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "connectivity_manager.h"
#include "system_registry.h"
#include "system_config.h"
#include "system_event.h"
#include "nvs_storage.h"
#include "web_server.h"

static const char *TAG = "CONN_MGR";

static connectivity_config_t s_config;
static connectivity_state_t s_state = CONNECTIVITY_IDLE;
static bool s_running = false;
static uint32_t s_wifi_stable_counter = 0;
#define WIFI_STABILITY_THRESHOLD_SEC 5

static int8_t rssi_to_bars(int8_t rssi) {
    if (rssi == 0) return 0;
    if (rssi >= -55) return 4;
    if (rssi >= -65) return 3;
    if (rssi >= -75) return 2;
    if (rssi >= -85) return 1;
    return 0;
}

/**
 * @brief Initialize GPIOs for button and status LED
 */
static void init_hardware_gpios(void) {
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << CONFIG_PIN_PROVISION_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&btn_conf);

    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << CONFIG_PIN_CONFIG_LED),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_conf);
    gpio_set_level(CONFIG_PIN_CONFIG_LED, 0);
}

/**
 * @brief Start Provisioning (AP Mode)
 */
static void start_provisioning(void) {
    ESP_LOGW(TAG, "Entering Provisioning Mode (AP: ESMU-Setup)...");
    s_state = CONNECTIVITY_PROVISIONING;
    system_report_event(SYSTEM_EVENT_START_CONFIGURATION);
    system_registry_set_subtext("ESMU-Setup\n192.168.4.1");
    
    wifi_manager_init(&s_config.wifi_config);
    wifi_manager_start_ap("ESMU-Setup", NULL);
    web_server_start();
}

/**
 * @brief Connectivity Orchestrator Task
 */
static void connectivity_manager_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Connectivity Orchestrator Task started.");
    init_hardware_gpios();
    
    bool mqtt_started = false;
    bool provisioned = false;
    uint32_t button_hold_ms = 0;
    bool led_state = false;
    uint32_t uptime_ticks = 0;

    /* Check for credentials in NVS */
    char ssid[33] = {0}, pass[64] = {0};
    if (nvs_storage_load_wifi_creds(ssid, sizeof(ssid), pass, sizeof(pass)) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded WiFi credentials from NVS. Connecting...");
        static char static_ssid[33], static_pass[64];
        strncpy(static_ssid, ssid, 32);
        strncpy(static_pass, pass, 63);
        s_config.wifi_config.ssid = static_ssid;
        s_config.wifi_config.password = static_pass;

        ESP_LOGI(TAG, "Starting WiFi STA -> SSID: [%s], Password: [%s]", s_config.wifi_config.ssid, s_config.wifi_config.password);
        
        wifi_manager_init(&s_config.wifi_config);
        wifi_manager_start_sta();
        provisioned = true;
    } else {
        start_provisioning();
        provisioned = false;
    }

    while (s_running) {
        /* 1. Handle Manual Provisioning Trigger (GPIO 15 Hold 5s) */
        int btn_level = gpio_get_level(CONFIG_PIN_PROVISION_BUTTON);
        if (btn_level == 0) { // Pressed (Low)
            button_hold_ms += 100;
            if (button_hold_ms % 1000 == 0) {
                ESP_LOGI(TAG, "Button holding: %ld ms", button_hold_ms);
            }
            if (button_hold_ms >= 5000) {
                if (s_state != CONNECTIVITY_PROVISIONING) {
                    ESP_LOGW(TAG, "Button held for 5s. Triggering Manual Provisioning (AP Mode)...");
                    if (mqtt_started) {
                        mqtt_manager_stop();
                        mqtt_started = false;
                    }
                    web_server_stop(); 
                    wifi_manager_stop();
                    vTaskDelay(pdMS_TO_TICKS(500));
                    
                    start_provisioning();
                    provisioned = false;
                } else {
                    ESP_LOGW(TAG, "Button held for 5s. Switching back to Station Mode...");
                    web_server_stop();
                    wifi_manager_stop();
                    gpio_set_level(CONFIG_PIN_CONFIG_LED, 0);
                    vTaskDelay(pdMS_TO_TICKS(500));
                    
                    wifi_manager_init(&s_config.wifi_config);
                    wifi_manager_start_sta();
                    provisioned = true;
                    system_report_event(SYSTEM_EVENT_DEVICE_CONFIGURATED);
                    s_state = CONNECTIVITY_IDLE;
                }
                button_hold_ms = 0; // Reset after action
            }
        } else {
            button_hold_ms = 0;
        }

        /* 2. Handle Provisioning Loop & LED Blinking */
        if (!provisioned) {
            // Blink LED (GPIO 4)
            led_state = !led_state;
            gpio_set_level(CONFIG_PIN_CONFIG_LED, led_state);

            char new_ssid[33] = {0}, new_pass[64] = {0};
            if (web_server_get_credentials(new_ssid, new_pass)) {
                ESP_LOGI(TAG, "New WiFi credentials retrieved -> SSID: %s, Pass: %s", new_ssid, new_pass);
                nvs_storage_save_wifi_creds(new_ssid, new_pass);
                
                web_server_stop();
                wifi_manager_stop();
                gpio_set_level(CONFIG_PIN_CONFIG_LED, 0); 

                static char static_ssid[33], static_pass[64];
                strncpy(static_ssid, new_ssid, 32);
                strncpy(static_pass, new_pass, 63);
                s_config.wifi_config.ssid = static_ssid;
                s_config.wifi_config.password = static_pass;
                
                vTaskDelay(pdMS_TO_TICKS(1000));

                ESP_LOGI(TAG, "Starting WiFi STA -> SSID: [%s], Password: [%s]", s_config.wifi_config.ssid, s_config.wifi_config.password);

                wifi_manager_init(&s_config.wifi_config);
                wifi_manager_start_sta();
                provisioned = true;
                system_report_event(SYSTEM_EVENT_DEVICE_CONFIGURATED);
                s_state = CONNECTIVITY_IDLE;
            }
        } else {
            /* 3. Normal Connection Loop (Logic moved here but runs every 100ms) */
            // Every 10 loops = 1 second
            if (uptime_ticks % 10 == 0) {
                bool wifi_connected = wifi_manager_is_connected();
                bool mqtt_connected = mqtt_manager_is_connected();
                int8_t rssi = 0;

                if (!wifi_connected) {
                    if (mqtt_started) {
                        mqtt_manager_stop();
                        mqtt_started = false;
                    }
                    s_wifi_stable_counter = 0;
                    s_state = CONNECTIVITY_IDLE;
                    system_registry_set_subtext("WiFi Connecting...");
                } else {
                    if (!mqtt_started) {
                        s_wifi_stable_counter++;
                        s_state = CONNECTIVITY_RECOVERING;
                        system_registry_set_subtext("WiFi Stable. Connecting MQTT...");

                        if (s_wifi_stable_counter >= WIFI_STABILITY_THRESHOLD_SEC) {
                            if (mqtt_manager_start() == ESP_OK) {
                                mqtt_started = true;
                            } else {
                                s_wifi_stable_counter = 0; 
                            }
                        }
                    } else {
                        if (mqtt_connected) {
                            s_state = CONNECTIVITY_FULL;
                            system_registry_set_subtext("Connected");
                        } else {
                            s_state = CONNECTIVITY_RECOVERING;
                            system_registry_set_subtext("MQTT Reconnecting...");
                        }
                    }
                    rssi = connectivity_manager_get_rssi();
                }
                system_registry_update_wifi(wifi_connected ? rssi_to_bars(rssi) : 0, rssi, mqtt_connected);
            }
        }

        uptime_ticks++;
        vTaskDelay(pdMS_TO_TICKS(100)); // Poll every 100ms
    }

    if (mqtt_started) mqtt_manager_stop();
    vTaskDelete(NULL);
}

esp_err_t connectivity_manager_init(const connectivity_config_t *config)
{
    if (config == NULL) return ESP_ERR_INVALID_ARG;
    s_config = *config;
    nvs_storage_init();
    mqtt_manager_init(&s_config.mqtt_config);
    ESP_LOGI(TAG, "Connectivity Manager initialized.");
    return ESP_OK;
}

esp_err_t connectivity_manager_start(void)
{
    if (s_running) return ESP_ERR_INVALID_STATE;
    s_running = true;
    BaseType_t ret = xTaskCreate(connectivity_manager_task, "conn_mgr_task", 4096, NULL, 5, NULL);
    if (ret != pdPASS) {
        s_running = false;
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t connectivity_manager_stop(void)
{
    s_running = false;
    return ESP_OK;
}

bool connectivity_manager_is_ready(void)
{
    return (s_state == CONNECTIVITY_FULL);
}

int8_t connectivity_manager_get_rssi(void)
{
    int8_t rssi = 0;
    wifi_manager_get_rssi(&rssi);
    return rssi;
}

connectivity_state_t connectivity_manager_get_state(void)
{
    return s_state;
}
