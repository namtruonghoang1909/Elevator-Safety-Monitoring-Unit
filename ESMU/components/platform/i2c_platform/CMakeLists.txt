set (srcs i2c_platform.c)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES driver freertos log
                    REQUIRES driver)