set (srcs st7789.c st7789_fonts.c)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES spi_platform pwm_platform driver freertos log
                    REQUIRES spi_platform pwm_platform)
