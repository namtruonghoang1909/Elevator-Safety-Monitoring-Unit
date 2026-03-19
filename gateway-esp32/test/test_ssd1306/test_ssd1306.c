#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "unity.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "i2c_bsp.h"
#include "ssd1306.h"

static const char *TAG = "TEST_SSD1306";

#define SDA_PIN         21
#define SCL_PIN         22
#define TEST_LED_PIN    2

static uint8_t bus_id = 0xFF;
static uint8_t ssd1306_dev_id = 0xFF;

// ─────────────────────────────────────────────
// Test Helpers
// ─────────────────────────────────────────────

static void wait_for_user(const char *message) {
    ESP_LOGI(TAG, ">>> INTERACTIVE TEST: %s", message);
    ESP_LOGI(TAG, ">>> Blink LED (GPIO %d) - Verify visually...", TEST_LED_PIN);
    
    bool led_state = false;
    for (int i = 0; i < 10; i++) {
        gpio_set_level(TEST_LED_PIN, led_state);
        led_state = !led_state;
        vTaskDelay(pdMS_TO_TICKS(300));
    }
    gpio_set_level(TEST_LED_PIN, 0);
}

// ─────────────────────────────────────────────
// Setup & Teardown
// ─────────────────────────────────────────────

void setUp(void) {
    if (bus_id == 0xFF) {
        TEST_ASSERT_EQUAL(ESP_OK, i2c_bsp_bus_init(&bus_id, (gpio_num_t)SDA_PIN, (gpio_num_t)SCL_PIN));
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << TEST_LED_PIN),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
    }

    if (ssd1306_dev_id == 0xFF) {
        ssd1306_config_t dev_cfg = {
            .address = SSD1306_I2C_ADDR,
            .name = "SSD1306_TEST",
            .bus_id = bus_id,
            .contrast = SSD1306_CONTRAST_DEFAULT
        };
        TEST_ASSERT_EQUAL(ESP_OK, ssd1306_init(&dev_cfg, &ssd1306_dev_id));
    }
}

void tearDown(void) {
}

// ─────────────────────────────────────────────
// Automated Tests
// ─────────────────────────────────────────────

void test_ssd1306_initialization(void) {
    TEST_ASSERT_NOT_EQUAL(0xFF, ssd1306_dev_id);
}

void test_ssd1306_commands(void) {
    TEST_ASSERT_EQUAL(ESP_OK, ssd1306_set_contrast(ssd1306_dev_id, 0x00));
    TEST_ASSERT_EQUAL(ESP_OK, ssd1306_set_contrast(ssd1306_dev_id, 0xFF));
    TEST_ASSERT_EQUAL(ESP_OK, ssd1306_set_contrast(ssd1306_dev_id, SSD1306_CONTRAST_DEFAULT));
    TEST_ASSERT_EQUAL(ESP_OK, ssd1306_invert_display(ssd1306_dev_id, true));
    vTaskDelay(pdMS_TO_TICKS(100));
    TEST_ASSERT_EQUAL(ESP_OK, ssd1306_invert_display(ssd1306_dev_id, false));
    TEST_ASSERT_EQUAL(ESP_OK, ssd1306_display_off(ssd1306_dev_id));
    vTaskDelay(pdMS_TO_TICKS(100));
    TEST_ASSERT_EQUAL(ESP_OK, ssd1306_display_on(ssd1306_dev_id));
}

void test_ssd1306_bounds_handling(void) {
    esp_err_t ret = ssd1306_set_cursor(ssd1306_dev_id, 8, 128); 
    ESP_LOGI(TAG, "Bounds check (8, 128) return: %s", esp_err_to_name(ret));
    ret = ssd1306_set_cursor(ssd1306_dev_id, 255, 255);
    ESP_LOGI(TAG, "Bounds check (255, 255) return: %s", esp_err_to_name(ret));
}

// ─────────────────────────────────────────────
// Interactive Tests (Bit & Pixel Precision)
// ─────────────────────────────────────────────

void test_ssd1306_visual_bit_markers(void) {
    ssd1306_clear(ssd1306_dev_id);
    uint8_t top_line[128];
    memset(top_line, 0x01, 128); 
    ssd1306_write_page(ssd1306_dev_id, 0, top_line);
    uint8_t bot_line[128];
    memset(bot_line, 0x80, 128);
    ssd1306_write_page(ssd1306_dev_id, 7, bot_line);
    wait_for_user("Verify Markers: SINGLE PIXEL line at very TOP and very BOTTOM of screen.");
}

void test_ssd1306_visual_staircase(void) {
    ssd1306_clear(ssd1306_dev_id);
    uint8_t ramp[128];
    for (int i = 0; i < 128; i++) {
        ramp[i] = (1 << (i / 16)); 
    }
    ssd1306_write_page(ssd1306_dev_id, 3, ramp);
    wait_for_user("Verify Staircase: Steps stepping DOWN from Left to Right (Bit 0 -> Bit 7).");
}

/**
 * @test Checkerboard pattern using the same logic for all pages.
 * Since each page is 8 bits (even), the pattern naturally alternates vertically.
 */
void test_ssd1306_visual_checkerboard(void) {
    uint8_t pattern[128];

    for (int i = 0; i < 128; i++) {
        // Horizontal alternation: 0xAA (10101010) and 0x55 (01010101)
        pattern[i] = (i % 2 == 0) ? 0xAA : 0x55;
    }

    for (int page = 0; page < 8; page++) {
        // Use the same pattern for all pages to ensure vertical continuity
        ssd1306_write_page(ssd1306_dev_id, page, pattern);
    }

    wait_for_user("Verify Checkerboard: Clean, uniform grid across all pages.");
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ssd1306_initialization);
    RUN_TEST(test_ssd1306_commands);
    RUN_TEST(test_ssd1306_bounds_handling);
    RUN_TEST(test_ssd1306_visual_bit_markers);
    RUN_TEST(test_ssd1306_visual_staircase);
    RUN_TEST(test_ssd1306_visual_checkerboard);
    UNITY_END();
}
