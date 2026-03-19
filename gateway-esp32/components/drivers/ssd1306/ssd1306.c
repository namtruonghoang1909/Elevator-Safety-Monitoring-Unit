#include "ssd1306.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "SSD1306";

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

static esp_err_t ssd1306_send_command(uint8_t dev_id, uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};  // Co=0, D/C#=0 → command mode
    return i2c_bsp_write_bytes(dev_id, buf, 2);
}

static esp_err_t ssd1306_send_commands(uint8_t dev_id, const uint8_t *cmds, size_t len)
{
    uint8_t buf[1 + len];
    buf[0] = 0x00;  // command mode
    for (size_t i = 0; i < len; i++) {
        buf[i + 1] = cmds[i];
    }
    return i2c_bsp_write_bytes(dev_id, buf, len + 1);
}

// ─────────────────────────────────────────────
// Core APIs
// ─────────────────────────────────────────────

esp_err_t ssd1306_init(const ssd1306_config_t *cfg, uint8_t *dev_id_out)
{
    if (cfg == NULL || dev_id_out == NULL) {
        ESP_LOGE(TAG, "Invalid arguments: cfg or dev_id_out is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    *dev_id_out = 255;  // invalid marker

    // Register device on the bus
    esp_err_t ret = i2c_bsp_add_device(dev_id_out,
                                   cfg->address,
                                   cfg->name ? cfg->name : "SSD1306",
                                   cfg->scl_speed_hz,
                                   cfg->bus_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SSD1306 device to I2C HAL");
        return ret;
    }

    ESP_LOGI(TAG, "SSD1306 registered with dev_id = %d", *dev_id_out);

    // Give hardware time to stabilize after power-on
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Initializing SSD1306 (dev_id=%d)", *dev_id_out);

    // Get contrast from config or use default
    uint8_t contrast = (cfg->contrast != 0) ? cfg->contrast : SSD1306_CONTRAST_DEFAULT;

    // Standard init sequence (from datasheet + common practice)
    const uint8_t init_sequence[] = {
        0xAE,               // 1. Display OFF
        0xD5, 0x80,         // 2. Set display clock divide ratio / osc freq
        0xA8, 0x3F,         // 3. Set multiplex ratio (64-1 = 0x3F)
        0xD3, 0x00,         // 4. Set display offset = 0
        0x40 | 0x00,        // 5. Set start line = 0
        0x8D, 0x14,         // 6. Charge pump enable (0x14 = enable)
        0x20, 0x00,         // 7. Memory addressing mode = Horizontal
        0xA1,               // 8. Segment remap (column 127 → SEG0)
        0xC8,               // 9. COM output scan direction reverse
        0xDA, 0x12,         // 10. COM pins HW config (alternative + no remap)
        0x81, contrast,    // 11. Set contrast (configurable)
        0xD9, 0xF1,         // 12. Set pre-charge period
        0xDB, 0x40,         // 13. Set VCOMH deselect level
        0xA4,               // 14. Entire display ON (RAM content)
        0xA6,               // 15. Normal display (not inverted)
        0xAF                // 16. Display ON
    };

    ret = ssd1306_send_commands(*dev_id_out, init_sequence, sizeof(init_sequence));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Init sequence failed: %s", esp_err_to_name(ret));
        i2c_bsp_remove_device(*dev_id_out);
        *dev_id_out = 255;
        return ret;
    }

    // Clear display after init
    ret = ssd1306_clear(*dev_id_out);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Clear failed after init: %s", esp_err_to_name(ret));
        i2c_bsp_remove_device(*dev_id_out);
        *dev_id_out = 255;
        return ret;
    }

    ESP_LOGI(TAG, "SSD1306 initialized successfully (contrast=0x%02X)", contrast);
    return ESP_OK;
}

esp_err_t ssd1306_deinit(uint8_t dev_id)
{
    ESP_LOGI(TAG, "Deinitializing SSD1306 (dev_id=%d)", dev_id);
    return i2c_bsp_remove_device(dev_id);
}

// ─────────────────────────────────────────────
// Display Control
// ─────────────────────────────────────────────

esp_err_t ssd1306_display_on(uint8_t dev_id)
{
    return ssd1306_send_command(dev_id, 0xAF);
}

esp_err_t ssd1306_display_off(uint8_t dev_id)
{
    return ssd1306_send_command(dev_id, 0xAE);
}

esp_err_t ssd1306_set_contrast(uint8_t dev_id, uint8_t contrast)
{
    esp_err_t ret;
    ret = ssd1306_send_command(dev_id, 0x81);
    if (ret != ESP_OK) return ret;
    return ssd1306_send_command(dev_id, contrast);
}

esp_err_t ssd1306_invert_display(uint8_t dev_id, bool invert)
{
    // 0xA6 = normal, 0xA7 = inverted
    return ssd1306_send_command(dev_id, invert ? 0xA7 : 0xA6);
}

// ─────────────────────────────────────────────
// Display Content
// ─────────────────────────────────────────────

esp_err_t ssd1306_clear(uint8_t dev_id)
{
    esp_err_t ret;

    // Set horizontal addressing mode
    ret = ssd1306_send_command(dev_id, 0x20);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, 0x00);  // horizontal mode
    if (ret != ESP_OK) return ret;

    // Column address range: 0 → 127
    ret = ssd1306_send_command(dev_id, 0x21);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, 0x00);           // start col
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, SSD1306_WIDTH-1); // end col
    if (ret != ESP_OK) return ret;

    // Page address range: 0 → 7
    ret = ssd1306_send_command(dev_id, 0x22);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, 0x00);  // start page
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, 0x07);  // end page
    if (ret != ESP_OK) return ret;

    // Fill 8 pages × 128 columns with 0x00
    uint8_t zero_line[128] = {0};
    for (int page = 0; page < SSD1306_PAGES; page++) {
        ret = ssd1306_set_cursor(dev_id, page, 0);
        if (ret != ESP_OK) return ret;

        ret = ssd1306_write_data(dev_id, zero_line, SSD1306_WIDTH);
        if (ret != ESP_OK) return ret;
    }

    return ESP_OK;
}

esp_err_t ssd1306_fill_screen(uint8_t dev_id, uint8_t color)
{
    uint8_t line[128];
    memset(line, color ? 0xFF : 0x00, 128);

    for (int page = 0; page < SSD1306_PAGES; page++) {
        esp_err_t ret = ssd1306_write_page(dev_id, page, line);
        if (ret != ESP_OK) return ret;
    }
    return ESP_OK;
}

esp_err_t ssd1306_set_cursor(uint8_t dev_id, uint8_t page, uint8_t col)
{
    esp_err_t ret;

    // Page address (0xB0 to 0xB7)
    ret = ssd1306_send_command(dev_id, 0xB0 | (page & 0x07));
    if (ret != ESP_OK) return ret;

    // Lower column address (0x00–0x0F)
    ret = ssd1306_send_command(dev_id, 0x00 | (col & 0x0F));
    if (ret != ESP_OK) return ret;

    // Higher column address (0x10–0x1F)
    ret = ssd1306_send_command(dev_id, 0x10 | ((col >> 4) & 0x0F));
    if (ret != ESP_OK) return ret;

    return ESP_OK;
}

esp_err_t ssd1306_write_data(uint8_t dev_id, const uint8_t *data, size_t len)
{
    // Send control byte + data in ONE I2C transaction
    // SSD1306 expects: [Co=0, D/C#=1] followed by pixel data in single transfer
    uint8_t buf[1 + len];
    buf[0] = 0x40;  // Control byte: Co=0, D/C#=1 (data mode)

    // Copy pixel data after control byte
    for (size_t i = 0; i < len; i++) {
        buf[i + 1] = data[i];
    }

    esp_err_t ret = i2c_bsp_write_bytes(dev_id, buf, len + 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write %u data bytes: %s", (unsigned)len, esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t ssd1306_write_page(uint8_t dev_id, uint8_t page, const uint8_t *data)
{
    esp_err_t ret = ssd1306_set_cursor(dev_id, page, 0);
    if (ret != ESP_OK) return ret;
    return ssd1306_write_data(dev_id, data, SSD1306_WIDTH);
}

// ─────────────────────────────────────────────
// Scrolling
// ─────────────────────────────────────────────

esp_err_t ssd1306_scroll_horizontal(uint8_t dev_id, ssd1306_scroll_dir_t direction,
                                    uint8_t start_page, uint8_t end_page, uint8_t speed)
{
    esp_err_t ret;
    ret = ssd1306_send_command(dev_id, 0x26 | direction);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, 0x00);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, start_page);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, speed & 0x07);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, end_page);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, 0x00);
    if (ret != ESP_OK) return ret;
    ret = ssd1306_send_command(dev_id, 0xFF);
    if (ret != ESP_OK) return ret;
    return ssd1306_send_command(dev_id, 0x2F);
}

esp_err_t ssd1306_scroll_stop(uint8_t dev_id)
{
    return ssd1306_send_command(dev_id, 0x2E);
}
