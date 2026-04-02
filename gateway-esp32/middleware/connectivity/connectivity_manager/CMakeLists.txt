idf_component_register(SRCS "connectivity_manager.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver wifi_manager mqtt_manager web_server core registry can_protocol nvs_platform)

