FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(
    SRCS ${app_sources}
    REQUIRES "system" "i2c_platform" "connectivity_manager" "motion_monitor" "nvs_flash" "wifi_manager"
)
