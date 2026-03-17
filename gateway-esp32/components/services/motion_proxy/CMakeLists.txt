set(srcs
    "motion_proxy.c"
    "src/core/task.c"
)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include" "src"
                    PRIV_INCLUDE_DIRS "src"
                    REQUIRES freertos system can_platform can_protocol)
