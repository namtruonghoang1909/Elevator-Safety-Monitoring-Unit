set(srcs
    "motion_proxy.c"
    "src/core/task.c"
)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include"
                    REQUIRES freertos core registry can_platform can_protocol telemetry_service)
