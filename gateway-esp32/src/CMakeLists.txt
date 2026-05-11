FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(
    SRCS ${app_sources}
    REQUIRES core spiffs ota_manager web_server
    INCLUDE_DIRS "." "${CMAKE_SOURCE_DIR}/middleware/connectivity/ota_manager/include"
)
