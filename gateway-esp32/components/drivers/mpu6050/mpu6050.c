#include <math.h>

#include "mpu6050.h"
#include "mpu6050_reg.h"
#include "i2c_platform.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MPU6050";

// ─────────────────────────────────────────────
// Global sensitivity & last config (used for verification/recovery)
// ─────────────────────────────────────────────
static float accel_sensitivity = MPU6050_ACCEL_SENS_2G;
static float gyro_sensitivity  = MPU6050_GYRO_SENS_250;

static mpu6050_config_t last_cfg = {0};

// ─────────────────────────────────────────────
// Private (static) function declarations
// ─────────────────────────────────────────────

static esp_err_t mpu6050_set_defaults(uint8_t dev_id);
static esp_err_t mpu6050_apply_config(uint8_t dev_id, const mpu6050_config_t *cfg);
static esp_err_t mpu6050_verify_registers(uint8_t dev_id);

// ─────────────────────────────────────────────
// Private (static) function definitions
// ─────────────────────────────────────────────

/**
 * @brief Apply default power and control settings (wake up, enable sensors, disable FIFO/master)
 */
static esp_err_t mpu6050_set_defaults(uint8_t dev_id)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Applying defaults: wake-up + X gyro PLL");
    ret = i2c_write_reg(dev_id, MPU6050_REG_PWR_MGMT_1, 0x01);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_LOGI(TAG, "Enabling all sensors (PWR_MGMT_2 = 0x00)");
    ret = i2c_write_reg(dev_id, MPU6050_REG_PWR_MGMT_2, 0x00);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Disabling FIFO & I2C master (USER_CTRL = 0x00)");
    ret = i2c_write_reg(dev_id, MPU6050_REG_USER_CTRL, 0x00);
    if (ret != ESP_OK) return ret;

    return ESP_OK;
}

/**
 * @brief Apply user-provided configuration (sample rate, DLPF, gyro & accel full-scale)
 */
static esp_err_t mpu6050_apply_config(uint8_t dev_id, const mpu6050_config_t *cfg)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Setting SMPLRT_DIV = 0x%02X", cfg->sample_rate_div);
    ret = i2c_write_reg(dev_id, MPU6050_REG_SMPLRT_DIV, cfg->sample_rate_div);
    if (ret != ESP_OK) return ret;

    uint8_t dlpf = cfg->enable_digital_filter ? 0x03 : 0x00;
    ESP_LOGI(TAG, "Setting CONFIG = 0x%02X (DLPF)", dlpf);
    ret = i2c_write_reg(dev_id, MPU6050_REG_CONFIG, dlpf);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Setting GYRO_CONFIG = 0x%02X", cfg->gyro_full_scale);
    ret = i2c_write_reg(dev_id, MPU6050_REG_GYRO_CONFIG, cfg->gyro_full_scale);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Setting ACCEL_CONFIG = 0x%02X", cfg->accel_full_scale);
    ret = i2c_write_reg(dev_id, MPU6050_REG_ACCEL_CONFIG, cfg->accel_full_scale);
    if (ret != ESP_OK) return ret;

    // Optional read-back (many registers are write-only)
    uint8_t verify = 0;
    ret = i2c_read_reg(dev_id, MPU6050_REG_ACCEL_CONFIG, &verify);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "ACCEL_CONFIG read-back = 0x%02X (wrote 0x%02X)", verify, cfg->accel_full_scale);
    } else {
        ESP_LOGW(TAG, "Cannot read back ACCEL_CONFIG: %s", esp_err_to_name(ret));
    }

    // Update sensitivities
    switch (cfg->accel_full_scale) {
        case MPU6050_ACCEL_FS_2G:  accel_sensitivity = MPU6050_ACCEL_SENS_2G;  break;
        case MPU6050_ACCEL_FS_4G:  accel_sensitivity = MPU6050_ACCEL_SENS_4G;  break;
        case MPU6050_ACCEL_FS_8G:  accel_sensitivity = MPU6050_ACCEL_SENS_8G;  break;
        case MPU6050_ACCEL_FS_16G: accel_sensitivity = MPU6050_ACCEL_SENS_16G; break;
        default:
            accel_sensitivity = MPU6050_ACCEL_SENS_2G;
            ESP_LOGW(TAG, "Unknown accel full-scale 0x%02X → defaulting to ±2g", cfg->accel_full_scale);
    }

    switch (cfg->gyro_full_scale) {
        case MPU6050_GYRO_FS_250_DPS:  gyro_sensitivity = MPU6050_GYRO_SENS_250;  break;
        case MPU6050_GYRO_FS_500_DPS:  gyro_sensitivity = MPU6050_GYRO_SENS_500;  break;
        case MPU6050_GYRO_FS_1000_DPS: gyro_sensitivity = MPU6050_GYRO_SENS_1000; break;
        case MPU6050_GYRO_FS_2000_DPS: gyro_sensitivity = MPU6050_GYRO_SENS_2000; break;
        default:
            gyro_sensitivity = MPU6050_GYRO_SENS_250;
            ESP_LOGW(TAG, "Unknown gyro full-scale 0x%02X → defaulting to ±250 °/s", cfg->gyro_full_scale);
    }

    ESP_LOGI(TAG, "Applied config → accel_sens=%.0f LSB/g, gyro_sens=%.1f LSB/°/s",
             accel_sensitivity, gyro_sensitivity);

    last_cfg = *cfg;

    return ESP_OK;
}

/**
 * @brief Verify configuration registers match expected values and attempt recovery if mismatch
 * 
 * This is a private helper — only used internally.
 */
static esp_err_t mpu6050_verify_registers(uint8_t dev_id)
{
    typedef struct {
        uint8_t  reg;
        uint8_t  expected;
        const char *name;
    } reg_check_t;

    reg_check_t checks[] = {
        {MPU6050_REG_PWR_MGMT_1,   0x01, "PWR_MGMT_1 (wake + X PLL)"},
        {MPU6050_REG_PWR_MGMT_2,   0x00, "PWR_MGMT_2 (all sensors on)"},
        {MPU6050_REG_USER_CTRL,    0x00, "USER_CTRL (no FIFO/master)"},
        {MPU6050_REG_SMPLRT_DIV,   last_cfg.sample_rate_div, "SMPLRT_DIV"},
        {MPU6050_REG_CONFIG,       last_cfg.enable_digital_filter ? 0x03 : 0x00, "CONFIG (DLPF)"},
        {MPU6050_REG_GYRO_CONFIG,  last_cfg.gyro_full_scale, "GYRO_CONFIG"},
        {MPU6050_REG_ACCEL_CONFIG, last_cfg.accel_full_scale, "ACCEL_CONFIG"},
    };

    const int num_checks = sizeof(checks) / sizeof(checks[0]);
    const int max_retries_per_reg = 3;

    ESP_LOGI(TAG, "Verifying %d configuration registers...", num_checks);

    bool all_ok = true;

    for (int i = 0; i < num_checks; i++) {
        uint8_t read_val = 0;
        esp_err_t ret = i2c_read_reg(dev_id, checks[i].reg, &read_val);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Cannot read %s (0x%02X): %s", checks[i].name, checks[i].reg, esp_err_to_name(ret));
            all_ok = false;
            continue;
        }

        if (read_val == checks[i].expected) {
            ESP_LOGD(TAG, "%s (0x%02X) = 0x%02X ✓", checks[i].name, checks[i].reg, read_val);
            continue;
        }

        ESP_LOGW(TAG, "%s (0x%02X): expected 0x%02X, got 0x%02X → attempting recovery",
                 checks[i].name, checks[i].reg, checks[i].expected, read_val);

        bool recovered = false;

        for (int retry = 1; retry <= max_retries_per_reg; retry++) {
            ESP_LOGI(TAG, "  Retry %d/%d: writing 0x%02X to 0x%02X",
                     retry, max_retries_per_reg, checks[i].expected, checks[i].reg);

            ret = i2c_write_reg(dev_id, checks[i].reg, checks[i].expected);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "  Write failed: %s", esp_err_to_name(ret));
                vTaskDelay(pdMS_TO_TICKS(20));
                continue;
            }

            vTaskDelay(pdMS_TO_TICKS(10));

            uint8_t verify = 0;
            ret = i2c_read_reg(dev_id, checks[i].reg, &verify);
            if (ret == ESP_OK && verify == checks[i].expected) {
                ESP_LOGI(TAG, "  Recovery success: %s now = 0x%02X", checks[i].name, verify);
                recovered = true;
                break;
            } else {
                ESP_LOGW(TAG, "  Still wrong after retry: got 0x%02X", verify);
            }
        }

        if (!recovered) {
            ESP_LOGE(TAG, "Failed to recover %s after %d attempts", checks[i].name, max_retries_per_reg);
            all_ok = false;
        }
    }

    if (all_ok) {
        ESP_LOGI(TAG, "All configuration registers verified or recovered successfully");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "One or more registers could not be verified/recovered");
        return ESP_ERR_INVALID_STATE;
    }
}

// ─────────────────────────────────────────────
// Public API Implementation
// ─────────────────────────────────────────────
// ────────────────────────────────────
// Initialization, Reset , Existance
// ────────────────────────────────────
esp_err_t mpu6050_init(const mpu6050_config_t *cfg, uint8_t *dev_id_out)
{
    if (cfg == NULL || dev_id_out == NULL) {
        ESP_LOGE(TAG, "Invalid arguments: cfg or dev_id_out is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    *dev_id_out = 255;  // invalid marker

    // Register device on the bus
    esp_err_t ret = i2c_add_device(dev_id_out,
                                   cfg->address,
                                   cfg->name ? cfg->name : "MPU6050",
                                   cfg->scl_speed_hz,
                                   cfg->bus_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add MPU6050 device to I2C HAL");
        return ret;
    }

    ESP_LOGI(TAG, "MPU6050 registered with dev_id = %d", *dev_id_out);

    // Apply default power/control settings
    ret = mpu6050_set_defaults(*dev_id_out);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to apply default settings");
        i2c_remove_device(*dev_id_out);
        *dev_id_out = 255;
        return ret;
    }

    // Apply user configuration
    ret = mpu6050_apply_config(*dev_id_out, cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to apply user configuration");
        i2c_remove_device(*dev_id_out);
        *dev_id_out = 255;
        return ret;
    }

    // Final presence check
    if (!mpu6050_is_present(*dev_id_out)) {
        ESP_LOGE(TAG, "MPU6050 WHO_AM_I verification failed");
        i2c_remove_device(*dev_id_out);
        *dev_id_out = 255;
        return ESP_ERR_NOT_FOUND;
    }

    // Verify all configuration registers (auto-recovery if needed)
    ret = mpu6050_verify_registers(*dev_id_out);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Configuration verification failed even after recovery attempts");
        i2c_remove_device(*dev_id_out);
        *dev_id_out = 255;
        return ret;
    }

    ESP_LOGI(TAG, "MPU6050 initialized and fully verified (dev_id: %d)", *dev_id_out);
    return ESP_OK;
}

esp_err_t mpu6050_reset(uint8_t dev_id)
{
    esp_err_t ret = i2c_write_reg(dev_id, MPU6050_REG_PWR_MGMT_1, 0x80); // reset bit
    if (ret == ESP_OK) {
        vTaskDelay(pdMS_TO_TICKS(100)); // wait for reset
    }
    return ret;
}

bool mpu6050_is_present(uint8_t dev_id)
{
    uint8_t who_am_i = 0;
    esp_err_t ret = i2c_read_reg(dev_id, MPU6050_REG_WHO_AM_I, &who_am_i);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read WHO_AM_I from dev %d: %s", dev_id, esp_err_to_name(ret));
        return false;
    }

    if (who_am_i != MPU6050_WHO_AM_I_VALUE) {
        ESP_LOGE(TAG, "WHO_AM_I mismatch: got 0x%02X, expected 0x%02X", who_am_i, MPU6050_WHO_AM_I_VALUE);
        return false;
    }

    ESP_LOGI(TAG, "WHO_AM_I register value matched (0x%02X)", who_am_i);
    return true;
}

// ────────────────────────────────────
// Data Reading
// ────────────────────────────────────
esp_err_t mpu6050_read_raw(uint8_t dev_id, mpu6050_raw_data_t *data)
{
    if (data == NULL) {
        ESP_LOGE(TAG, "Invalid data pointer");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buffer[14];
    esp_err_t ret = i2c_read_consecutive_regs(dev_id,
                                              MPU6050_REG_ACCEL_XOUT_H,
                                              buffer, 14);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Burst read failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Big-endian → combine high + low bytes
    data->accel_x     = (int16_t)((buffer[0] << 8) | buffer[1]);
    data->accel_y     = (int16_t)((buffer[2] << 8) | buffer[3]);
    data->accel_z     = (int16_t)((buffer[4] << 8) | buffer[5]);
    data->temperature = (int16_t)((buffer[6] << 8) | buffer[7]);
    data->gyro_x      = (int16_t)((buffer[8]  << 8) | buffer[9]);
    data->gyro_y      = (int16_t)((buffer[10] << 8) | buffer[11]);
    data->gyro_z      = (int16_t)((buffer[12] << 8) | buffer[13]);

    return ESP_OK;
}

esp_err_t mpu6050_read_scaled(uint8_t dev_id, mpu6050_scaled_data_t *data)
{
    mpu6050_raw_data_t raw;
    esp_err_t ret = mpu6050_read_raw(dev_id, &raw);
    if (ret != ESP_OK) return ret;

    data->accel_x_g = (float)raw.accel_x / accel_sensitivity;
    data->accel_y_g = (float)raw.accel_y / accel_sensitivity;
    data->accel_z_g = (float)raw.accel_z / accel_sensitivity;

    data->gyro_x_dps = (float)raw.gyro_x / gyro_sensitivity;
    data->gyro_y_dps = (float)raw.gyro_y / gyro_sensitivity;
    data->gyro_z_dps = (float)raw.gyro_z / gyro_sensitivity;

    data->temperature_c = ((float)raw.temperature / MPU6050_TEMP_SENS) + MPU6050_TEMP_OFFSET;

    return ESP_OK;
}

/**
 * @brief Get magnitude of acceleration vector (useful for motion/free-fall detection)
 */
float mpu6050_get_accel_magnitude(const mpu6050_scaled_data_t *data)
{
    return sqrtf(data->accel_x_g * data->accel_x_g +
                 data->accel_y_g * data->accel_y_g +
                 data->accel_z_g * data->accel_z_g);
}