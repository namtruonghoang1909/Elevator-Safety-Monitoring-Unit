#include "st7789.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pwm_platform.h"
#include "spi_platform.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "st7789";

// ─────────────────────────────────────────────
// Private Variables
// ─────────────────────────────────────────────
static st7789_config_t _cfg;
static uint8_t         _spi_dev_id;
static uint8_t         _rotation = 0;

// ─────────────────────────────────────────────
// Private Helpers
// ─────────────────────────────────────────────

static void write_cmd(uint8_t cmd)
{
    gpio_set_level(_cfg.dc_pin, 0);
    platform_spi_transfer(_spi_dev_id, &cmd, 1);
}

static void write_data(const uint8_t *data, size_t len)
{
    gpio_set_level(_cfg.dc_pin, 1);
    platform_spi_transfer(_spi_dev_id, data, len);
}

static void write_data_byte(uint8_t data)
{
    write_data(&data, 1);
}

// ─────────────────────────────────────────────
// Global APIs
// ─────────────────────────────────────────────

esp_err_t st7789_init(const st7789_config_t *cfg)
{
    _cfg = *cfg;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << _cfg.dc_pin) | (1ULL << _cfg.rst_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    esp_err_t ret = platform_spi_add_device(&_spi_dev_id, _cfg.spi_host, _cfg.cs_pin, _cfg.clk_speed_hz, 3, "ST7789");
    if (ret != ESP_OK) return ret;

    if (_cfg.bl_pin != GPIO_NUM_NC) {
        platform_pwm_init(_cfg.bl_pin, 5000, LEDC_TIMER_8_BIT, LEDC_LOW_SPEED_MODE, LEDC_TIMER_1, LEDC_CHANNEL_1);
        platform_pwm_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    }


    ESP_LOGI(TAG, "Resetting display...");
    gpio_set_level(_cfg.rst_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(_cfg.rst_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    write_cmd(0x01); // SW Reset
    vTaskDelay(pdMS_TO_TICKS(10));

    write_cmd(0x11); // Sleep Out
    vTaskDelay(pdMS_TO_TICKS(120));

    write_cmd(0x3A); // Color Mode
    write_data_byte(0x05); // RGB565

    st7789_set_rotation(0);

    write_cmd(0x21); // Inversion ON
    write_cmd(0x13); // Normal Display ON

    st7789_fill_screen(ST7789_COLOR_BLACK);

    write_cmd(0x29); // Display ON
    vTaskDelay(pdMS_TO_TICKS(20));

    if (_cfg.bl_pin != GPIO_NUM_NC) {
        platform_pwm_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 80);
    }

    ESP_LOGI(TAG, "ST7789 Initialized");
    return ESP_OK;
}

void st7789_set_rotation(uint8_t m)
{
    write_cmd(0x36);
    _rotation = m % 4;
    switch (_rotation) {
        case 0: write_data_byte(0x00); break;
        case 1: write_data_byte(0x60); break;
        case 2: write_data_byte(0xC0); break;
        case 3: write_data_byte(0xA0); break;
    }
}

esp_err_t st7789_set_backlight(uint8_t brightness_percent)
{
    if (_cfg.bl_pin == GPIO_NUM_NC) return ESP_ERR_INVALID_STATE;
    return platform_pwm_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, brightness_percent);
}

esp_err_t st7789_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t data[4];
    write_cmd(0x2A);
    data[0] = x1 >> 8; data[1] = x1 & 0xFF;
    data[2] = x2 >> 8; data[3] = x2 & 0xFF;
    write_data(data, 4);

    write_cmd(0x2B);
    data[0] = y1 >> 8; data[1] = y1 & 0xFF;
    data[2] = y2 >> 8; data[3] = y2 & 0xFF;
    write_data(data, 4);

    write_cmd(0x2C);
    return ESP_OK;
}

esp_err_t st7789_push_colors(const uint16_t *colors, uint32_t len)
{
    gpio_set_level(_cfg.dc_pin, 1);
    return platform_spi_transfer(_spi_dev_id, (const uint8_t *)colors, len * 2);
}

esp_err_t st7789_push_colors_dma(const uint16_t *colors, uint32_t len)
{
    gpio_set_level(_cfg.dc_pin, 1);
    return platform_spi_queue_transfer(_spi_dev_id, (const uint8_t *)colors, len * 2);
}


esp_err_t st7789_fill_screen(uint16_t color)
{
    return st7789_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

esp_err_t st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return ESP_ERR_INVALID_ARG;
    if ((x + w) > ST7789_WIDTH) w = ST7789_WIDTH - x;
    if ((y + h) > ST7789_HEIGHT) h = ST7789_HEIGHT - y;

    st7789_set_window(x, y, x + w - 1, y + h - 1);
    uint16_t swapped_color = (color << 8) | (color >> 8);
    
    #define FILL_BUF_SIZE 240
    uint16_t line[FILL_BUF_SIZE];
    for (int i = 0; i < FILL_BUF_SIZE; i++) line[i] = swapped_color;

    uint32_t pixels = w * h;
    uint32_t full_chunks = pixels / FILL_BUF_SIZE;
    uint32_t rem_pixels = pixels % FILL_BUF_SIZE;

    for (uint32_t i = 0; i < full_chunks; i++) st7789_push_colors(line, FILL_BUF_SIZE);
    if (rem_pixels > 0) st7789_push_colors(line, rem_pixels);

    return ESP_OK;
}

esp_err_t st7789_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    st7789_draw_fast_hline(x, y, w, color);
    st7789_draw_fast_hline(x, y + h - 1, w, color);
    st7789_draw_fast_vline(x, y, h, color);
    st7789_draw_fast_vline(x + w - 1, y, h, color);
    return ESP_OK;
}

esp_err_t st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return ESP_ERR_INVALID_ARG;
    st7789_set_window(x, y, x, y);
    uint16_t swapped_color = (color << 8) | (color >> 8);
    return st7789_push_colors(&swapped_color, 1);
}

void st7789_draw_fast_hline(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
    st7789_fill_rect(x, y, w, 1, color);
}

void st7789_draw_fast_vline(uint16_t x, uint16_t y, uint16_t h, uint16_t color)
{
    st7789_fill_rect(x, y, 1, h, color);
}

esp_err_t st7789_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    if (x1 == x2) { st7789_draw_fast_vline(x1, (y1 < y2 ? y1 : y2), abs(y2 - y1) + 1, color); return ESP_OK; }
    if (y1 == y2) { st7789_draw_fast_hline((x1 < x2 ? x1 : x2), y1, abs(x2 - x1) + 1, color); return ESP_OK; }

    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1, sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        st7789_draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
    return ESP_OK;
}

void st7789_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
    st7789_draw_pixel(x0, y0 + r, color);
    st7789_draw_pixel(x0, y0 - r, color);
    st7789_draw_pixel(x0 + r, y0, color);
    st7789_draw_pixel(x0 - r, y0, color);
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        st7789_draw_pixel(x0 + x, y0 + y, color);
        st7789_draw_pixel(x0 - x, y0 + y, color);
        st7789_draw_pixel(x0 + x, y0 - y, color);
        st7789_draw_pixel(x0 - x, y0 - y, color);
        st7789_draw_pixel(x0 + y, y0 + x, color);
        st7789_draw_pixel(x0 - y, y0 + x, color);
        st7789_draw_pixel(x0 + y, y0 - x, color);
        st7789_draw_pixel(x0 - y, y0 - x, color);
    }
}

void st7789_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    st7789_draw_fast_vline(x0, y0 - r, 2 * r + 1, color);
    int f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        st7789_draw_fast_vline(x0 + x, y0 - y, 2 * y + 1, color);
        st7789_draw_fast_vline(x0 - x, y0 - y, 2 * y + 1, color);
        st7789_draw_fast_vline(x0 + y, y0 - x, 2 * x + 1, color);
        st7789_draw_fast_vline(x0 - y, y0 - x, 2 * x + 1, color);
    }
}

void st7789_draw_bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *bitmap, uint16_t color, uint16_t bg)
{
    st7789_set_window(x, y, x + w - 1, y + h - 1);
    uint16_t color_swap = (color << 8) | (color >> 8);
    uint16_t bg_swap = (bg << 8) | (bg >> 8);
    uint16_t line[w];
    int byte_width = (w + 7) / 8;
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            if (bitmap[j * byte_width + i / 8] & (128 >> (i & 7))) line[i] = color_swap;
            else line[i] = bg_swap;
        }
        st7789_push_colors(line, w);
    }
}

void st7789_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    // Note: data is expected to be already big-endian or swap must be done
    st7789_set_window(x, y, x + w - 1, y + h - 1);
    st7789_push_colors(data, w * h);
}

void st7789_draw_char(uint16_t x, uint16_t y, char c, const st7789_font_t *font, uint16_t color, uint16_t bg)
{
    if (c < 32 || c > 126) return;
    uint32_t offset = (c - 32) * font->height;
    const uint8_t *char_data = &font->data[offset];
    st7789_draw_bitmap(x, y, font->width, font->height, char_data, color, bg);
}

void st7789_draw_string(uint16_t x, uint16_t y, const char *str, const st7789_font_t *font, uint16_t color, uint16_t bg)
{
    while (*str) {
        st7789_draw_char(x, y, *str, font, color, bg);
        x += font->width;
        if (x + font->width > ST7789_WIDTH) { x = 0; y += font->height; }
        str++;
    }
}
