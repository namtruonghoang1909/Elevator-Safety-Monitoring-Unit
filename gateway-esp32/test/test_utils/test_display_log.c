#include "test_display_log.h"
#include "display_service.h"
#include "system_registry.h"
#include "nvs_flash.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define MAX_TEST_LINES 4
#define LINE_LEN 24

static char s_log_lines[MAX_TEST_LINES][LINE_LEN];
static int s_line_count = 0;

static void update_registry_with_log(void) {
    char full_msg[128] = {0};
    int offset = 0;
    for (int i = 0; i < s_line_count; i++) {
        int written = snprintf(full_msg + offset, sizeof(full_msg) - offset, "%s\n", s_log_lines[i]);
        if (written > 0) offset += written;
    }
    system_registry_set_subtext(full_msg);
}

void test_display_log_init(void) {
    // 1. Init NVS if needed
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // 2. Init Registry and Display
    system_registry_init();
    display_service_init();
    
    // 3. Set state to TESTING
    system_registry_set_state(SYSTEM_STATE_TESTING);
    
    test_display_log_clear();
}

void test_display_log(const char *fmt, ...) {
    char new_line[LINE_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(new_line, sizeof(new_line), fmt, args);
    va_end(args);

    // Scroll if full
    if (s_line_count >= MAX_TEST_LINES) {
        for (int i = 0; i < MAX_TEST_LINES - 1; i++) {
            memcpy(s_log_lines[i], s_log_lines[i+1], LINE_LEN);
        }
        memcpy(s_log_lines[MAX_TEST_LINES - 1], new_line, LINE_LEN);
    } else {
        memcpy(s_log_lines[s_line_count], new_line, LINE_LEN);
        s_line_count++;
    }

    update_registry_with_log();
}

void test_display_log_clear(void) {
    memset(s_log_lines, 0, sizeof(s_log_lines));
    s_line_count = 0;
    system_registry_set_subtext("");
}
