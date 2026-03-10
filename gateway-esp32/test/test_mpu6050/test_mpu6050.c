#include <unity.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mpu6050.h"
#include "i2c_platform.h"
#include "driver/gpio.h"

static const char *TAG = "MPU6050_TEST";

// ─────────────────────────────────────────────
// Test Constants & Pinout
// ─────────────────────────────────────────────
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define MPU6050_ADDR 0x68
#define STATUS_LED_GPIO 15

static uint8_t bus_id = 255;
static uint8_t dev_id = 255;

/**
 * @brief Interaction Signal: Rapidly blinks LED to alert user to move hardware.
 * @param seconds Duration of the blink sequence.
 */
void led_blink_interaction_signal(int seconds) {
    for (int i = 0; i < seconds * 4; i++) { // 250ms period (4Hz)
        gpio_set_level(STATUS_LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(125));
        gpio_set_level(STATUS_LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(125));
    }
}

// ─────────────────────────────────────────────
// Unity Setup & Teardown
// ─────────────────────────────────────────────

void setUp(void) {
    if (bus_id == 255) {
        // Initialize I2C Bus once per test session
        ESP_ERROR_CHECK(i2c_bus_init(&bus_id, I2C_SDA_PIN, I2C_SCL_PIN));
        
        // Initialize Status LED on GPIO 15
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << STATUS_LED_GPIO),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
        gpio_set_level(STATUS_LED_GPIO, 0); // Default to OFF
    }
}

void tearDown(void) {
    // Ensure LED is OFF and device handle is cleaned up to prevent leaks
    gpio_set_level(STATUS_LED_GPIO, 0); 
    if (dev_id != 255) {
        i2c_remove_device(dev_id);
        dev_id = 255;
    }
}

// ─────────────────────────────────────────────
// Test Cases
// ─────────────────────────────────────────────

/**
 * @test Verify I2C communication and MPU6050 identity.
 */
void test_mpu6050_communication(void) {
    ESP_LOGI(TAG, "Scanning I2C bus %d...", bus_id);
    uint8_t addr_list[10];
    size_t found_count = 0;
    i2c_scan(bus_id, addr_list, 10, &found_count);
    
    ESP_LOGI(TAG, "Found %d devices:", found_count);
    for (int i = 0; i < found_count; i++) {
        ESP_LOGI(TAG, "  - 0x%02X", addr_list[i]);
    }

    mpu6050_config_t cfg = {
        .address = MPU6050_ADDR,
        .bus_id = bus_id,
        .scl_speed_hz = 400000,
        .accel_full_scale = MPU6050_ACCEL_FS_2G,
        .gyro_full_scale = MPU6050_GYRO_FS_250_DPS,
        .sample_rate_div = 0,
        .use_gyro_pll = true,
        .enable_digital_filter = true
    };

    TEST_ASSERT_EQUAL(ESP_OK, mpu6050_init(&cfg, &dev_id));
    TEST_ASSERT_TRUE(mpu6050_is_present(dev_id));
}

/**
 * @test Verify scaling factors across different Full Scale Ranges (FSR).
 * @protocol 
 *  1. LED BLINKS (5s) -> Place FLAT.
 *  2. LED OFF -> Samples taken (5s).
 */
void test_mpu6050_scaling_and_fsr(void) {
    mpu6050_config_t cfg = {
        .address = MPU6050_ADDR,
        .bus_id = bus_id,
        .scl_speed_hz = 400000,
        .accel_full_scale = MPU6050_ACCEL_FS_2G,
        .gyro_full_scale = MPU6050_GYRO_FS_250_DPS,
        .sample_rate_div = 0,
        .use_gyro_pll = true,
        .enable_digital_filter = true
    };

    ESP_LOGI(TAG, "Interaction: Place the board FLAT (LED blinking)...");
    led_blink_interaction_signal(5); // Signal waiting period
    // LED is now OFF during sampling phase

    // 1. Verify 2G Sensitivity (Expect ~16384 LSB per g)
    TEST_ASSERT_EQUAL(ESP_OK, mpu6050_init(&cfg, &dev_id));
    ESP_LOGI(TAG, "Checking 2G scaling...");
    for (int i = 0; i < 5; i++) {
        mpu6050_raw_data_t raw;
        TEST_ASSERT_EQUAL(ESP_OK, mpu6050_read_raw(dev_id, &raw));
        ESP_LOGI(TAG, "Sample %d: Raw Z = %d", i + 1, raw.accel_z);
        TEST_ASSERT_INT_WITHIN(2500, 16384, raw.accel_z);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // 2. Verify 16G Sensitivity (Expect ~2048 LSB per g)
    i2c_remove_device(dev_id);
    cfg.accel_full_scale = MPU6050_ACCEL_FS_16G;
    TEST_ASSERT_EQUAL(ESP_OK, mpu6050_init(&cfg, &dev_id));
    ESP_LOGI(TAG, "Checking 16G scaling...");
    for (int i = 0; i < 5; i++) {
        mpu6050_raw_data_t raw;
        TEST_ASSERT_EQUAL(ESP_OK, mpu6050_read_raw(dev_id, &raw));
        ESP_LOGI(TAG, "Sample %d: Raw Z = %d", i + 1, raw.accel_z);
        TEST_ASSERT_INT_WITHIN(400, 2048, raw.accel_z);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @test Verify physical orientation and gravity vector magnitude.
 * @protocol 
 *  1. Initial check (must be already flat).
 *  2. LED BLINKS (5s) -> Flip UPSIDE DOWN.
 *  3. LED OFF -> Samples taken (5s).
 */
void test_mpu6050_physical_vectors(void) {
    mpu6050_config_t cfg = {
        .address = MPU6050_ADDR,
        .bus_id = bus_id,
        .scl_speed_hz = 400000,
        .accel_full_scale = MPU6050_ACCEL_FS_2G,
        .gyro_full_scale = MPU6050_GYRO_FS_250_DPS,
        .sample_rate_div = 0,
        .use_gyro_pll = true,
        .enable_digital_filter = true
    };
    TEST_ASSERT_EQUAL(ESP_OK, mpu6050_init(&cfg, &dev_id));

    // Verify FLAT magnitude is ~1.0g
    mpu6050_scaled_data_t scaled;
    TEST_ASSERT_EQUAL(ESP_OK, mpu6050_read_scaled(dev_id, &scaled));
    float mag = mpu6050_get_accel_magnitude(&scaled);
    ESP_LOGI(TAG, "Stationary magnitude: %.3f g", mag);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, mag);
    TEST_ASSERT_TRUE(scaled.accel_z_g > 0.7f); // Z must be positive when flat

    ESP_LOGI(TAG, "Interaction: FLIP the board UPSIDE DOWN (LED blinking)...");
    led_blink_interaction_signal(5); // Signal waiting period
    // LED is now OFF during sampling phase

    // Verify UPSIDE DOWN readings
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQUAL(ESP_OK, mpu6050_read_scaled(dev_id, &scaled));
        ESP_LOGI(TAG, "Sample %d: Upside down Z = %.3f g", i + 1, scaled.accel_z_g);
        TEST_ASSERT_TRUE(scaled.accel_z_g < -0.7f); // Z must be negative when flipped
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @test Verify driver's ability to recover from register corruption.
 */
void test_mpu6050_robustness(void) {
    mpu6050_config_t cfg = {
        .address = MPU6050_ADDR,
        .bus_id = bus_id,
        .scl_speed_hz = 400000,
        .accel_full_scale = MPU6050_ACCEL_FS_2G,
        .gyro_full_scale = MPU6050_GYRO_FS_250_DPS,
        .sample_rate_div = 0,
        .use_gyro_pll = true,
        .enable_digital_filter = true
    };
    
    TEST_ASSERT_EQUAL(ESP_OK, mpu6050_init(&cfg, &dev_id));

    // Corrupt the GYRO_CONFIG register (write ±2000DPS when expecting ±250DPS)
    ESP_LOGI(TAG, "Corrupting GYRO_CONFIG register...");
    TEST_ASSERT_EQUAL(ESP_OK, i2c_write_reg(dev_id, MPU6050_REG_GYRO_CONFIG, 0x18));
    
    // Trigger verification/recovery by re-initializing
    ESP_LOGI(TAG, "Re-initializing to trigger auto-recovery...");
    i2c_remove_device(dev_id);
    TEST_ASSERT_EQUAL(ESP_OK, mpu6050_init(&cfg, &dev_id));
    
    // Read back to verify it was recovered to 0x00 (250DPS)
    uint8_t val;
    TEST_ASSERT_EQUAL(ESP_OK, i2c_read_reg(dev_id, MPU6050_REG_GYRO_CONFIG, &val));
    TEST_ASSERT_EQUAL(0x00, val);
    ESP_LOGI(TAG, "Recovery verified: GYRO_CONFIG restored to 0x%02X", val);
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mpu6050_communication);
    RUN_TEST(test_mpu6050_scaling_and_fsr);
    RUN_TEST(test_mpu6050_physical_vectors);
    RUN_TEST(test_mpu6050_robustness);
    UNITY_END();
}
