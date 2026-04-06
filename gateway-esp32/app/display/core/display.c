/**
 * @file display.c
 * @brief Main task and lifecycle for the display component
 */

#include "display.h"
#include "dashboard.h"
#include "config_screen.h"
#include "st7789.h"
#include "board_pins.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "system_registry.h"
#include "spi_platform.h"

static const char *TAG = "display";

// ─────────────────────────────────────────────
// Private Variables
// ─────────────────────────────────────────────
static TaskHandle_t _task_handle = NULL;
static bool _refresh_requested = false;

typedef enum {
    UI_MODE_DASHBOARD,
    UI_MODE_CONFIG,
} ui_mode_t;

// ─────────────────────────────────────────────
// Private Helpers
// ─────────────────────────────────────────────

static void display_task(void *pvParameters)
{
    system_status_registry_t snapshot;
    uint32_t emergency_start_tick = 0;
    uint32_t last_emergency_finish_tick = 0;
    bool showing_emergency = false;
    bool last_fault_active = false;
    ui_mode_t current_ui_mode = UI_MODE_DASHBOARD;
    system_state_id_t last_system_state = SYSTEM_STATE_IDLE;
    
    // Initial clear
    st7789_fill_screen(ST7789_COLOR_BLACK);
    
    // Get initial snapshot and draw full
    system_registry_get_snapshot(&snapshot);
    dashboard_draw_full(&snapshot, false);
    last_system_state = snapshot.current_state;

    while (1) {
        system_registry_get_snapshot(&snapshot);
        uint32_t now = xTaskGetTickCount();

        // 1. Detect transition to Emergency
        // Stability: Only trigger if not already showing AND cooldown passed
        if (snapshot.fault_active && !last_fault_active) {
            bool cooldown_passed = (last_emergency_finish_tick == 0) || 
                                   ((now - last_emergency_finish_tick) * portTICK_PERIOD_MS > 2000);
            
            if (!showing_emergency && cooldown_passed) {
                showing_emergency = true;
                emergency_start_tick = now;
                ESP_LOGW(TAG, "Emergency alert triggered! Showing for 5s.");
                dashboard_draw_full(&snapshot, true);
            }
        }
        
        last_fault_active = snapshot.fault_active;

        // 2. Handle State/Mode Transitions
        if (snapshot.current_state != last_system_state || _refresh_requested) {
            _refresh_requested = false;
            last_system_state = snapshot.current_state;

            if (snapshot.current_state == SYSTEM_STATE_CONFIGURING) {
                current_ui_mode = UI_MODE_CONFIG;
                config_screen_draw_full(&snapshot);
            } else {
                current_ui_mode = UI_MODE_DASHBOARD;
                dashboard_draw_full(&snapshot, false);
            }
        }

        // 3. Handle Display Logic
        if (showing_emergency) {
            // Emergency overlay always uses dashboard style (inverted)
            if ((now - emergency_start_tick) * portTICK_PERIOD_MS > 5000) {
                showing_emergency = false;
                last_emergency_finish_tick = now;
                ESP_LOGI(TAG, "Emergency alert timeout. Returning to previous mode.");
                
                // Redraw current mode after emergency
                if (current_ui_mode == UI_MODE_CONFIG) {
                    config_screen_draw_full(&snapshot);
                } else {
                    dashboard_draw_full(&snapshot, false);
                }
            } else {
                dashboard_update(&snapshot, true);
            }
        } else {
            // Update current UI mode
            if (current_ui_mode == UI_MODE_CONFIG) {
                config_screen_update(&snapshot);
            } else {
                dashboard_update(&snapshot, false);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

// ─────────────────────────────────────────────
// Global APIs
// ─────────────────────────────────────────────

esp_err_t display_init(void)
{
    st7789_config_t cfg = {
        .spi_host = SPI3_HOST,
        .cs_pin = ST7789_CS_PIN,
        .dc_pin = ST7789_DC_PIN,
        .rst_pin = ST7789_RST_PIN,
        .bl_pin = ST7789_BL_PIN,
        .clk_speed_hz = 40 * 1000 * 1000, // 40MHz
    };

    esp_err_t ret = st7789_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ST7789: %s", esp_err_to_name(ret));
        return ret;
    }

    st7789_set_rotation(0); 
    return ESP_OK;
}

esp_err_t display_start(void)
{
    if (_task_handle != NULL) return ESP_ERR_INVALID_STATE;

    xTaskCreate(display_task, "display_task", 4096, NULL, 5, &_task_handle);
    if (_task_handle == NULL) return ESP_FAIL;

    ESP_LOGI(TAG, "Display component started");
    return ESP_OK;
}

void display_set_brightness(uint8_t percent)
{
    st7789_set_backlight(percent);
}

void display_refresh(void)
{
    _refresh_requested = true;
}
