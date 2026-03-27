cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME edge-stm32)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
file(GLOB_RECURSE APP_SOURCES 
    "App/src/*.c"
    "App/modules/bsp/*.c"
    "App/modules/drivers/*.c"
    "App/modules/drivers/mpu6050/*.c"
    "App/modules/drivers/ssd1306/*.c"
    "App/modules/services/*.c"
    "App/modules/services/edge_logger/*.c"
    "App/modules/services/edge_telemetry/*.c"
    "App/modules/services/motion_monitor/*.c"
    "App/modules/services/watchdog/*.c"
    "App/modules/system/*.c"
    "App/modules/system/registry/*.c"
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${APP_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "App/inc"
    "App/modules/bsp"
    "App/modules/drivers"
    "App/modules/drivers/mpu6050"
    "App/modules/drivers/ssd1306"
    "App/modules/services"
    "App/modules/services/edge_logger"
    "App/modules/services/edge_telemetry"
    "App/modules/services/display_service"
    "App/modules/services/motion_monitor"
    "App/modules/services/motion_monitor/core"
    "App/modules/services/motion_monitor/kinematics"
    "App/modules/services/motion_monitor/filters"
    "App/modules/services/motion_monitor/abnormal_detector"
    "App/modules/services/watchdog"
    "App/modules/system"
    "App/modules/system/registry"
    "../shared/can_protocol"
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
