set(SOURCES "ota_manager.c")
set(INCLUDE_DIRS "include")

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS "include"
                       REQUIRES esp_https_ota esp_http_client)
