FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(
    SRCS ${app_sources}
    REQUIRES "system" "i2c_bsp" "connectivity_manager" "motion_proxy" "telemetry" "can_bsp" "can_protocol" "nvs_flash" "wifi_manager" freertos
)
