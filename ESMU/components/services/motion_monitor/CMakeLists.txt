set(srcs
    "service.c"
    "src/core/task.c"
    "src/core/processor.c"
    "src/filter/filter.c"
    "src/fsm/fsm.c"
    "src/balance/balance.c"
)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include" "src"
                    PRIV_INCLUDE_DIRS "src"
                    REQUIRES mpu6050 freertos system)
