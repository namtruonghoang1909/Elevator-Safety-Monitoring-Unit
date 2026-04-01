set (srcs pwm_bsp.c)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES driver log
                    REQUIRES driver)
