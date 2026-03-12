cmake_minimum_required(VERSION 3.16.0)

list(APPEND EXTRA_COMPONENT_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/components/system"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/drivers/ssd1306"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/platform/i2c_platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/display"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/motion_monitor"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/connectivity/connectivity_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/connectivity/mqtt_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/connectivity/wifi_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/connectivity/web_server"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESMU)
