#include <unity.h>
#include "esmu_protocol.h"
#include <string.h>

void setUp(void) {
    // No hardware setup needed for protocol struct verification
}

void tearDown(void) {
    // No tear down needed
}

void test_ele_health_struct_size(void) {
    TEST_ASSERT_EQUAL_INT(8, sizeof(ele_health_t));
}

void test_ele_emergency_struct_size(void) {
    TEST_ASSERT_EQUAL_INT(8, sizeof(ele_emergency_t));
}

void test_edge_heartbeat_struct_size(void) {
    TEST_ASSERT_EQUAL_INT(8, sizeof(edge_heartbeat_t));
}

void test_ele_health_packing(void) {
    ele_health_t packet;
    memset(&packet, 0, sizeof(packet));
    
    // Test offsets implicitly by ensuring size is correct and fields fit
    // This is more of a sanity check that we can access fields without crash/misalignment
    packet.avg_tilt = 100;
    packet.max_tilt = 200;
    packet.balance = BALANCE_STATE_TILT_LEFT;
    packet.health_score = 95;
    
    TEST_ASSERT_EQUAL_INT16(100, packet.avg_tilt);
    TEST_ASSERT_EQUAL_INT16(200, packet.max_tilt);
    TEST_ASSERT_EQUAL_UINT8(BALANCE_STATE_TILT_LEFT, packet.balance);
    TEST_ASSERT_EQUAL_UINT8(95, packet.health_score);
}

void test_ele_emergency_packing(void) {
    ele_emergency_t packet;
    memset(&packet, 0, sizeof(packet));
    
    packet.fault_code = FAULT_SHAKE;
    packet.severity = 5;
    packet.fault_value = 1234;
    
    TEST_ASSERT_EQUAL_UINT8(FAULT_SHAKE, packet.fault_code);
    TEST_ASSERT_EQUAL_UINT8(5, packet.severity);
    TEST_ASSERT_EQUAL_INT16(1234, packet.fault_value);
}

void test_edge_heartbeat_packing(void) {
    edge_heartbeat_t packet;
    memset(&packet, 0, sizeof(packet));
    
    packet.edge_health = EDGE_HEALTH_OK;
    packet.edge_state = EDGE_STATE_RUNNING;
    packet.uptime_sec = 3600;
    
    TEST_ASSERT_EQUAL_UINT8(EDGE_HEALTH_OK, packet.edge_health);
    TEST_ASSERT_EQUAL_UINT8(EDGE_STATE_RUNNING, packet.edge_state);
    TEST_ASSERT_EQUAL_UINT32(3600, packet.uptime_sec);
}

int run_protocol_tests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ele_health_struct_size);
    RUN_TEST(test_ele_emergency_struct_size);
    RUN_TEST(test_edge_heartbeat_struct_size);
    RUN_TEST(test_ele_health_packing);
    RUN_TEST(test_ele_emergency_packing);
    RUN_TEST(test_edge_heartbeat_packing);
    return UNITY_END();
}

/**
 *  app_main is required for PlatformIO to run tests on ESP32
 */
void app_main(void) {
    run_protocol_tests();
}
