set(srcs
    "core/display.c"
    "ui/widgets.c"
    "ui/dashboard.c"
    "ui/config_screen.c"
)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include" "core" "ui"
                    REQUIRES st7789 registry board_pins)
