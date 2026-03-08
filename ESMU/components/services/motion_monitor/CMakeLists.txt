set(srcs
    "motion_monitor.c"
    "src/core/motion_task.c"
    "src/core/motion_processor.c"
    "src/filter/motion_filter.c"
    "src/fsm/motion_fsm.c"
    "src/balance/motion_balance.c"
)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "include" "src"
                    PRIV_INCLUDE_DIRS "src"
                    REQUIRES mpu6050 freertos)
