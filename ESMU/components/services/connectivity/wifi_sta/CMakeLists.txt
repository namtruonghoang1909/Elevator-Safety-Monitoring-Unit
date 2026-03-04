idf_component_register(SRCS "wifi_sta.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi nvs_flash esp_common log esp_system esp_event esp_netif)

