set (srcs st7789.c st7789_fonts.c)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES spi_bsp pwm_bsp driver freertos log
                    REQUIRES spi_bsp pwm_bsp)
