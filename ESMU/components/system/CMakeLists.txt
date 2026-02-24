set(srcs "system.c"
        "controller/system_controller.c"
)

set (include_dirs "include")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${include_dirs}
                       REQUIRES freertos                    # FreeRTOS.h, task.h, queue.h, portMAX_DELAY, pdMS_TO_TICKS
                                log                         # ESP_LOGI, ESP_LOGE, ESP_LOGW, TAG
                                esp_common)