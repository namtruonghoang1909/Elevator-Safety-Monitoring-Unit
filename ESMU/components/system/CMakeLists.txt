set(srcs "system.c"
        "controller/system_controller.c"
        "error/system_error.c"
        "event/system_event.c"
)

set (include_dirs "include")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${include_dirs}
                       PRIV_REQUIRES esp_event nvs_flash freertos 
)