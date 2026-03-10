/**
 * @file display_service.c
 * @brief Layer 3: Display Service Manager Implementation
 */

#include "display_service.h"
#include "ssd1306.h"
#include "system_registry.h"
#include "src/core/primitives.h"
#include "src/ui/ui_components.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "DISP_SVC";

static uint8_t g_dev_id = 0;
static uint8_t g_fb[1024]; 
static TaskHandle_t g_refresh_task_handle = NULL;
static bool g_is_initialized = false;

static void display_refresh_task(void *pvParameters) {
    system_status_registry_t snap;
    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t frame_count = 0;

    while (1) {
        system_registry_get_snapshot(&snap);
        display_primitives_clear(g_fb);
        
        // 3. Global Inverse Toggle (Error phase uses inverse video)
        esp_err_t err = ssd1306_invert_display(g_dev_id, (snap.current_state == SYSTEM_STATE_ERROR));
        if (err != ESP_OK && (frame_count % 100 == 0)) {
            ESP_LOGE(TAG, "Failed to invert display: %s", esp_err_to_name(err));
        }

        // 4. Render Layout based on System State
        switch (snap.current_state) {
            case SYSTEM_STATE_INITIALIZING:
                ui_draw_initializing_view(g_fb, snap.sub_status);
                break;

            case SYSTEM_STATE_MONITORING:
                ui_draw_header(g_fb, snap.wifi_level, snap.mqtt_connected);
                ui_draw_footer(g_fb, snap.uptime_sec);
                ui_draw_monitoring_view(g_fb, snap.motion_state, snap.balance_state, snap.elevator_health);
                break;

            case SYSTEM_STATE_CONFIGURING:
                ui_draw_header(g_fb, snap.wifi_level, snap.mqtt_connected);
                ui_draw_footer(g_fb, snap.uptime_sec);
                ui_draw_configuring_view(g_fb, snap.sub_status);
                break;

            case SYSTEM_STATE_ERROR:
                ui_draw_error_view(g_fb, snap.sub_status);
                break;

            default:
                ui_draw_footer(g_fb, snap.uptime_sec);
                display_draw_string(g_fb, 40, 30, "SYSTEM IDLE");
                break;
        }
        for (int p = 0; p < 8; p++) {
            err = ssd1306_write_page(g_dev_id, p, &g_fb[p * 128]);
            if (err != ESP_OK && (frame_count % 100 == 0)) {
                ESP_LOGE(TAG, "Failed to write page %d: %s", p, esp_err_to_name(err));
            }
        }

        frame_count++;
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(66));
    }
}

esp_err_t display_service_init(void) {
    if (g_is_initialized) return ESP_OK;

    ssd1306_config_t cfg = {
        .address = 0x3C,
        .name = "SSD1306",
        .scl_speed_hz = 400000,
        .bus_id = 0,
        .contrast = 0xCF
    };

    esp_err_t err = ssd1306_init(&cfg, &g_dev_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SSD1306 init failed: %s", esp_err_to_name(err));
        return err;
    }
    
    err = ssd1306_clear(g_dev_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SSD1306 clear failed: %s", esp_err_to_name(err));
    }

    xTaskCreate(display_refresh_task, "disp_refresh", 4096, NULL, 5, &g_refresh_task_handle);

    g_is_initialized = true;
    ESP_LOGI(TAG, "Display Service (Layered) initialized");
    return ESP_OK;
}

esp_err_t display_service_deinit(void) {
    if (!g_is_initialized) return ESP_OK;
    if (g_refresh_task_handle) vTaskDelete(g_refresh_task_handle);
    
    esp_err_t err = ssd1306_deinit(g_dev_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SSD1306 deinit failed: %s", esp_err_to_name(err));
    }
    
    g_is_initialized = false;
    return ESP_OK;
}
