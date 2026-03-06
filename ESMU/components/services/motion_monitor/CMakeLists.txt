idf_component_register(
    SRCS "motion_monitor.c" 
         "filter/motion_filter.c" 
         "fsm/motion_fsm.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "filter" "fsm"
    REQUIRES mpu6050 freertos log esp_common esp_system
)
