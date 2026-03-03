cmake_minimum_required(VERSION 3.16.0)

list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/components/system"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/components/drivers"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/components/platform"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/components/services/connectivity"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESMU)
