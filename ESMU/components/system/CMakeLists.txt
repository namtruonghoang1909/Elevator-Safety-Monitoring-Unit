set(srcs "system.c"
        "controller/system_controller.c"
)

set (include_dirs "include")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${include_dirs}
                       REQUIRES nvs_flash esp_common log freertos esp_system)
