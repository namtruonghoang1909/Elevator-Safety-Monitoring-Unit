#include "system.h"
#include "system_event.h"
#include "system_error.h"

#include "nvs_flash.h"
#include "esp_event.h"  
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

void system_init(void) {
    // Implementation for system initialization
    // This could involve setting up hardware, initializing state machines, etc.
    
    system_report_event(SYSTEM_EVENT_BOOT);
}


