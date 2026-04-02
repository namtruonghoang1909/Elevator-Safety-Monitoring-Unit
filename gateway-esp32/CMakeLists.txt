cmake_minimum_required(VERSION 3.16.0)

list(APPEND EXTRA_COMPONENT_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/board_pins"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/i2c_platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/spi_platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/pwm_platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/can_platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/uart_platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/nvs_platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/drivers/sim_a7680c"
    "${CMAKE_CURRENT_SOURCE_DIR}/drivers/ssd1306"
    "${CMAKE_CURRENT_SOURCE_DIR}/drivers/st7789"
    "${CMAKE_CURRENT_SOURCE_DIR}/middleware/connectivity/connectivity_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/middleware/connectivity/mqtt_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/middleware/connectivity/wifi_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/middleware/connectivity/web_server"
    "${CMAKE_CURRENT_SOURCE_DIR}/middleware/communication/telemetry_service"
    "${CMAKE_CURRENT_SOURCE_DIR}/middleware/communication/cellular_service"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/registry"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/motion_proxy"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/../shared/can_protocol"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESMU)
