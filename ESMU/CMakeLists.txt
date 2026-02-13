cmake_minimum_required(VERSION 3.16.0)

list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/components/drivers"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/components/services"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/components/system"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESMU)
