idf_component_register(
    SRCS "can_bsp.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_common log
)
