idf_component_register(
    SRCS "web_server.c" "web_api_handlers.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_server log spiffs registry json
)
