set (srcs ssd1306.c)

idf_component_register (SRCS "${srcs}"
                        INCLUDE_DIRS "include"
                        REQUIRES i2c_bsp esp_common freertos)