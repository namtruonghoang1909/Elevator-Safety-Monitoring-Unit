idf_component_register(
    SRCS 
        "display_service.c"
        "src/core/primitives.c"
        "src/ui/ui_components.c"
    INCLUDE_DIRS "include" "."
    REQUIRES "ssd1306" "system" "i2c_bsp" "freertos" "log"
)
