#include "pwm_bsp.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "pwm_bsp";

static ledc_timer_bit_t _resolution = LEDC_TIMER_8_BIT;

esp_err_t pwm_bsp_init(gpio_num_t gpio_num, uint32_t freq_hz, ledc_timer_bit_t resolution, ledc_mode_t speed_mode, ledc_timer_t timer_num, ledc_channel_t channel)
{
    _resolution = resolution;

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = speed_mode,
        .timer_num        = timer_num,
        .duty_resolution  = resolution,
        .freq_hz          = freq_hz,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    esp_err_t ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) return ret;

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = speed_mode,
        .channel        = channel,
        .timer_sel      = timer_num,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = gpio_num,
        .duty           = 0,
        .hpoint         = 0
    };
    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "PWM initialized on GPIO %d, freq %d Hz, res %d", gpio_num, (int)freq_hz, (int)resolution);
    return ESP_OK;
}

esp_err_t pwm_bsp_set_duty(ledc_mode_t speed_mode, ledc_channel_t channel, uint8_t duty_percent)
{
    if (duty_percent > 100) duty_percent = 100;

    uint32_t max_duty = (1 << _resolution) - 1;
    uint32_t duty = (duty_percent * max_duty) / 100;

    esp_err_t ret = ledc_set_duty(speed_mode, channel, duty);
    if (ret != ESP_OK) return ret;

    return ledc_update_duty(speed_mode, channel);
}
