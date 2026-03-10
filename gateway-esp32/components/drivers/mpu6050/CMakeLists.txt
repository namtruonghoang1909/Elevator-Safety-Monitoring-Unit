set (srcs mpu6050.c)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    REQUIRES i2c_platform driver log freertos)