cmake_minimum_required(VERSION 3.16.0)

list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/components/system"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/components/drivers/mpu6050"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/components/drivers/ssd1306"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/components/platform/i2c_platform"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESMU)
