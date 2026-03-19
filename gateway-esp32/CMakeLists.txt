cmake_minimum_required(VERSION 3.16.0)

list(APPEND EXTRA_COMPONENT_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/components/system"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/drivers/ssd1306"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/bsp/i2c_bsp"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/bsp/can_bsp"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/display"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/motion_proxy"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/communication/telemetry"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/connectivity/connectivity_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/connectivity/mqtt_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/connectivity/wifi_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/services/connectivity/web_server"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shared/can_protocol"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESMU)
