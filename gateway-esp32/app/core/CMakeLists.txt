set(srcs
    "boot/hardware/hardware_boot.c"
    "boot/system_boot.c"
    "controller/system_controller.c"
    "heart_beat/heartbeat.c"
    "system.c"
)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    REQUIRES registry board_pins connectivity_manager cellular_service telemetry_service motion_proxy can_platform i2c_platform uart_platform st7789 display spiffs)
