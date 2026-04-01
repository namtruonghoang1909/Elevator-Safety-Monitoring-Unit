/**
 * @file pwm_bsp.h
 * @brief Simple LEDC PWM abstraction for backlight and signal control
 */

#pragma once

#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

/**
 * @brief Initialize a PWM channel on a specific GPIO
 * 
 * @param gpio_num GPIO pin
 * @param freq_hz Frequency in Hz
 * @param resolution LEDC resolution (e.g. LEDC_TIMER_8_BIT)
 * @param speed_mode LEDC speed mode
 * @param timer_num LEDC timer
 * @param channel LEDC channel
 * @return esp_err_t 
 */
esp_err_t pwm_bsp_init(gpio_num_t gpio_num, uint32_t freq_hz, ledc_timer_bit_t resolution, ledc_mode_t speed_mode, ledc_timer_t timer_num, ledc_channel_t channel);

/**
 * @brief Set the duty cycle in percentage (0-100)
 * 
 * @param channel LEDC channel
 * @param duty_percent 0 to 100
 * @return esp_err_t 
 */
esp_err_t pwm_bsp_set_duty(ledc_mode_t speed_mode, ledc_channel_t channel, uint8_t duty_percent);
