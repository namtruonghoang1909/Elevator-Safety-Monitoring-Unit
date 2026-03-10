#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "esp_log.h"
#include "motion_monitor.h"

static const char *TAG = "TEST_MOTION_MONITOR";

/**
 * @brief Helper to inject a steady value for a number of samples
 * 1 sample = 10ms internally (if matching the task rate)
 */
static void inject_steady_z(float linear_z_g, uint32_t samples) {
    mpu6050_scaled_data_t sample = {0};
    // In TEST mode, gravity_offset_z is set to 1.0f.
    sample.accel_z_g = 1.0f + linear_z_g; 
    
    for (uint32_t i = 0; i < samples; i++) {
        motion_monitor_inject_sample(&sample);
    }
}

void setUp(void) {
    // Initialize in TEST mode (mpu_dev_id = 0xFF)
    motion_monitor_config_t cfg = {
        .mpu_dev_id = 0xFF,
        .filter_alpha = 1.0f, // No filtering for precise logic testing
    };
    motion_monitor_init(&cfg);
    motion_monitor_calibrate(); // Set baseline to 1.0g
}

void tearDown(void) {
}

/**
 * @brief Verify STATIONARY -> MOVING_UP -> DECELERATING_UP -> STATIONARY transition
 */
void test_trip_up_with_decel(void) {
    // 1. Start at Stationary
    inject_steady_z(0.0f, 5);
    TEST_ASSERT_EQUAL(MOTION_STATE_STATIONARY, motion_monitor_get_motion());

    // 2. Start Moving Up (+0.1g)
    inject_steady_z(0.10f, 25);
    TEST_ASSERT_EQUAL(MOTION_STATE_MOVING_UP, motion_monitor_get_motion());

    // 3. Braking while Up (-0.1g pulse)
    inject_steady_z(-0.10f, 25);
    TEST_ASSERT_EQUAL(MOTION_STATE_DECELERATING_UP, motion_monitor_get_motion());

    // 4. Back to Still
    inject_steady_z(0.0f, 25);
    TEST_ASSERT_EQUAL(MOTION_STATE_STATIONARY, motion_monitor_get_motion());
}

/**
 * @brief Verify STATIONARY -> MOVING_DOWN -> DECELERATING_DOWN -> STATIONARY transition
 */
void test_trip_down_with_decel(void) {
    // 1. Start Moving Down (-0.1g)
    inject_steady_z(-0.10f, 25);
    TEST_ASSERT_EQUAL(MOTION_STATE_MOVING_DOWN, motion_monitor_get_motion());

    // 2. Braking while Down (+0.1g pulse)
    inject_steady_z(0.10f, 25);
    TEST_ASSERT_EQUAL(MOTION_STATE_DECELERATING_DOWN, motion_monitor_get_motion());

    // 3. Back to Still
    inject_steady_z(0.0f, 25);
    TEST_ASSERT_EQUAL(MOTION_STATE_STATIONARY, motion_monitor_get_motion());
}

/**
 * @brief Verify that MOVING_UP can stop without a braking pulse (gradual slow)
 */
void test_trip_up_gradual_stop(void) {
    // 1. Moving Up
    inject_steady_z(0.10f, 25);
    TEST_ASSERT_EQUAL(MOTION_STATE_MOVING_UP, motion_monitor_get_motion());

    // 2. Gradual slow to 0.0g (No negative pulse)
    inject_steady_z(0.0f, 25);
    TEST_ASSERT_EQUAL(MOTION_STATE_STATIONARY, motion_monitor_get_motion());
}

/**
 * @brief Verify XY Shake Magnitude calculation
 */
void test_shake_magnitude(void) {
    mpu6050_scaled_data_t sample = {
        .accel_x_g = 0.3f,
        .accel_y_g = 0.4f,
        .accel_z_g = 1.0f
    };
    
    motion_monitor_inject_sample(&sample);
    
    motion_metrics_t m;
    motion_monitor_get_metrics(&m);
    
    // sqrt(0.3^2 + 0.4^2) = 0.5
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, m.shake_mag);
}

/**
 * @brief Verify Balance (Tilt) detection
 */
void test_balance_detection(void) {
    mpu6050_scaled_data_t sample = {0};
    sample.accel_z_g = 1.0f;
    motion_metrics_t m;

    // 1. Level
    sample.accel_x_g = 0.05f;
    sample.accel_y_g = -0.05f;
    motion_monitor_inject_sample(&sample);
    motion_monitor_get_metrics(&m);
    TEST_ASSERT_EQUAL(BALANCE_STATE_LEVEL, m.balance);
    TEST_ASSERT_EQUAL(BALANCE_STATE_LEVEL, motion_monitor_get_equilibrium());

    // 2. Tilt Left (-X)
    sample.accel_x_g = -0.20f;
    motion_monitor_inject_sample(&sample);
    motion_monitor_get_metrics(&m);
    TEST_ASSERT_EQUAL(BALANCE_STATE_TILT_LEFT, m.balance);
    TEST_ASSERT_EQUAL(BALANCE_STATE_TILT_LEFT, motion_monitor_get_equilibrium());

    // 3. Tilt Right (+X)
    sample.accel_x_g = 0.20f;
    motion_monitor_inject_sample(&sample);
    motion_monitor_get_metrics(&m);
    TEST_ASSERT_EQUAL(BALANCE_STATE_TILT_RIGHT, m.balance);
    TEST_ASSERT_EQUAL(BALANCE_STATE_TILT_RIGHT, motion_monitor_get_equilibrium());

    // 4. Tilt Forward (+Y)
    sample.accel_x_g = 0.0f;
    sample.accel_y_g = 0.20f;
    motion_monitor_inject_sample(&sample);
    motion_monitor_get_metrics(&m);
    TEST_ASSERT_EQUAL(BALANCE_STATE_TILT_FORWARD, m.balance);
    TEST_ASSERT_EQUAL(BALANCE_STATE_TILT_FORWARD, motion_monitor_get_equilibrium());

    // 5. Tilt Backward (-Y)
    sample.accel_y_g = -0.20f;
    motion_monitor_inject_sample(&sample);
    motion_monitor_get_metrics(&m);
    TEST_ASSERT_EQUAL(BALANCE_STATE_TILT_BACKWARD, m.balance);
    TEST_ASSERT_EQUAL(BALANCE_STATE_TILT_BACKWARD, motion_monitor_get_equilibrium());
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_trip_up_with_decel);
    RUN_TEST(test_trip_down_with_decel);
    RUN_TEST(test_trip_up_gradual_stop);
    RUN_TEST(test_shake_magnitude);
    RUN_TEST(test_balance_detection);
    UNITY_END();
}
