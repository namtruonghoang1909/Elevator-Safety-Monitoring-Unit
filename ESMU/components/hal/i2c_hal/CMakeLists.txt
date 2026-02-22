set (srcs i2c_hal.c)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_i2c)