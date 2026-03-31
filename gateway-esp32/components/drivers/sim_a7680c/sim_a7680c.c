/**
 * @file sim_a7680c.c
 * @brief Datasheet-compliant implementation for A76XX LTE modules.
 */

#include "sim_a7680c.h"
#include "uart_bsp.h"
#include "board_pins.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "SIM_A7680C";

#define SIM_LINE_BUF_SIZE   256
static SemaphoreHandle_t g_sim_mutex = NULL;

#define LOCK_SIM()   (xSemaphoreTake(g_sim_mutex, pdMS_TO_TICKS(15000)) == pdTRUE)
#define UNLOCK_SIM() (xSemaphoreGive(g_sim_mutex))

// ─────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────

/**
 * @brief Advanced terminal waiter that filters out URCs and captures specific lines.
 */
static esp_err_t wait_for_terminal(const char* expected_prefix, char* out_line, size_t out_len, uint32_t timeout_ms) {
    char line[SIM_LINE_BUF_SIZE];
    uint32_t start_tick = xTaskGetTickCount();
    bool found_expected = (expected_prefix == NULL);
    esp_err_t result = ESP_ERR_TIMEOUT;

    while ((xTaskGetTickCount() - start_tick) < pdMS_TO_TICKS(timeout_ms)) {
        int len = uart_bsp_read_line(line, sizeof(line), 500);
        if (len <= 0) continue;

        // Strip leading whitespace/junk if needed (simplified)
        char* trimmed = line;
        while(*trimmed == '\r' || *trimmed == '\n' || *trimmed == ' ') trimmed++;
        if (*trimmed == '\0') continue;

        ESP_LOGD(TAG, "RX: %s", trimmed);

        // Discard Echoes (Lines starting with AT)
        if (strncmp(trimmed, "AT", 2) == 0) continue;

        // Discard URCs and system messages
        if (strncmp(trimmed, "+CREG:", 6) == 0 || strncmp(trimmed, "+CEREG:", 7) == 0 || 
            strncmp(trimmed, "+CTZV:", 7) == 0 || strncmp(trimmed, "DST:", 4) == 0 ||
            strncmp(trimmed, "*ATREADY:", 9) == 0 || strncmp(trimmed, "SMS READY", 9) == 0 ||
            strncmp(trimmed, "PB DONE", 7) == 0) {
            continue;
        }

        if (strcmp(trimmed, "OK") == 0) {
            result = found_expected ? ESP_OK : ESP_FAIL;
            break;
        }
        if (strstr(trimmed, "ERROR") != NULL) {
            result = ESP_FAIL;
            break;
        }

        // Capture data line if it matches prefix
        if (expected_prefix != NULL && !found_expected) {
            if (strncmp(trimmed, expected_prefix, strlen(expected_prefix)) == 0) {
                found_expected = true;
                if (out_line && out_len > 0) {
                    strncpy(out_line, trimmed, out_len - 1);
                    out_line[out_len - 1] = '\0';
                }
            } else if (expected_prefix[0] == '\0') {
                // If prefix is empty string, we take the first non-empty, non-OK/ERROR line
                found_expected = true;
                if (out_line && out_len > 0) {
                    strncpy(out_line, trimmed, out_len - 1);
                    out_line[out_len - 1] = '\0';
                }
            }
        }
    }
    return result;
}

static esp_err_t internal_ping(void) {
    uart_bsp_flush();
    uart_bsp_send_str("AT\r\n");
    return wait_for_terminal(NULL, NULL, 0, 1000);
}

// ─────────────────────────────────────────────
// Global function definitions
// ─────────────────────────────────────────────

esp_err_t sim_a7680c_hw_reset(void) {
    ESP_LOGI(TAG, "Datasheet Pulse on GPIO %d...", SIM_RST_PIN);
    gpio_set_direction(SIM_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(SIM_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(2500)); // 2.5s pulse
    gpio_set_level(SIM_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10000)); // Boot wait
    return ESP_OK;
}

esp_err_t sim_a7680c_init(void) {
    if (g_sim_mutex == NULL) g_sim_mutex = xSemaphoreCreateMutex();
    if (!LOCK_SIM()) return ESP_ERR_TIMEOUT;

    ESP_LOGI(TAG, "SIM Init Sequence Started...");
    uart_bsp_set_data_cb(NULL);
    uart_bsp_flush();

    // 1. Baud Sync
    bool sync = false;
    for(int i=0; i<5; i++) {
        if (internal_ping() == ESP_OK) { sync = true; break; }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    if (!sync) { UNLOCK_SIM(); return ESP_FAIL; }

    // 2. Base Config (Datasheet Mandates)
    uart_bsp_send_str("ATE0\r\n");      // Echo Off
    wait_for_terminal(NULL, NULL, 0, 1000);
    uart_bsp_send_str("AT+CMEE=2\r\n");  // Verbose Errors
    wait_for_terminal(NULL, NULL, 0, 1000);
    uart_bsp_send_str("AT+CNMP=2\r\n");  // Preferred mode: Automatic (LTE/GSM)
    wait_for_terminal(NULL, NULL, 0, 1000);
    uart_bsp_send_str("AT+CFUN=1\r\n");  // Radio Full
    wait_for_terminal(NULL, NULL, 0, 2000);
    
    UNLOCK_SIM();
    ESP_LOGI(TAG, "Init OK.");
    return ESP_OK;
}

esp_err_t sim_a7680c_get_sim_info(sim_a7680c_info_t* info) {
    if (!info) return ESP_ERR_INVALID_ARG;
    if (!LOCK_SIM()) return ESP_ERR_TIMEOUT;
    
    // Force Echo Off again to be absolutely sure
    uart_bsp_send_str("ATE0\r\n");
    wait_for_terminal(NULL, NULL, 0, 500);

    // Clear struct to prevent garbage data
    memset(info, 0, sizeof(sim_a7680c_info_t));
    char line[SIM_LINE_BUF_SIZE];

    // Identify Hardware
    uart_bsp_send_str("ATI\r\n");
    if (wait_for_terminal("Model:", line, sizeof(line), 1000) == ESP_OK) {
        sscanf(line, "Model: %19s", info->model);
    }
    
    // Get IMEI (Try SIMEI then CGSN)
    uart_bsp_send_str("AT+SIMEI?\r\n");
    if (wait_for_terminal("+SIMEI:", line, sizeof(line), 1000) == ESP_OK) {
        char* p = strchr(line, ' '); if(p) strncpy(info->imei, p+1, 15);
    } else {
        uart_bsp_send_str("AT+CGSN\r\n");
        if (wait_for_terminal("", line, sizeof(line), 1000) == ESP_OK) {
            strncpy(info->imei, line, 15);
        }
    }
    info->imei[15] = '\0';

    // Get IMSI
    uart_bsp_send_str("AT+CIMI\r\n");
    if (wait_for_terminal("", line, sizeof(line), 1000) == ESP_OK) {
        strncpy(info->imsi, line, 15);
    }
    info->imsi[15] = '\0';

    // Get Mobile Number (Optional)
    uart_bsp_send_str("AT+CNUM\r\n");
    if (wait_for_terminal("+CNUM:", line, sizeof(line), 2000) == ESP_OK) {
        char* s = strchr(line, '\"');
        if (s) {
            char* next_quote = strchr(s + 1, '\"');
            if (next_quote) {
                char* num_start = strchr(next_quote + 1, '\"');
                if (num_start) {
                    char* num_end = strchr(num_start + 1, '\"');
                    if (num_end) {
                        *num_end = '\0';
                        strncpy(info->phone_number, num_start + 1, 19);
                    }
                }
            }
        }
    }

    // Get Operator
    uart_bsp_send_str("AT+COPS?\r\n");
    if (wait_for_terminal("+COPS:", line, sizeof(line), 2000) == ESP_OK) {
        char* s = strchr(line, '\"');
        if (s) {
            char* e = strchr(s+1, '\"');
            if (e) { *e = '\0'; strncpy(info->operator_name, s+1, 31); }
        }
    }

    // Get Signal
    uart_bsp_send_str("AT+CSQ\r\n");
    if (wait_for_terminal("+CSQ:", line, sizeof(line), 1000) == ESP_OK) {
        int rssi, ber;
        if (sscanf(line, "+CSQ: %d,%d", &rssi, &ber) == 2) {
            info->signal_strength = (rssi == 99) ? 0 : rssi;
        }
    }

    // Check Registration (Dual Mode)
    bool reg = false;
    uart_bsp_send_str("AT+CEREG?\r\n"); // Check LTE first
    if (wait_for_terminal("+CEREG:", line, sizeof(line), 1000) == ESP_OK) {
        int n, stat; if (sscanf(line, "+CEREG: %d,%d", &n, &stat) >= 2) if(stat==1 || stat==5) reg=true;
    }
    if (!reg) {
        uart_bsp_send_str("AT+CREG?\r\n");
        if (wait_for_terminal("+CREG:", line, sizeof(line), 1000) == ESP_OK) {
            int n, stat; if (sscanf(line, "+CREG: %d,%d", &n, &stat) >= 2) if(stat==1 || stat==5) reg=true;
        }
    }
    info->is_registered = reg;

    // Get Health (Voltage/Temp)
    uart_bsp_send_str("AT+CBC\r\n");
    if (wait_for_terminal("+CBC:", line, sizeof(line), 1000) == ESP_OK) {
        int b, p, v; sscanf(line, "+CBC: %d,%d,%d", &b, &p, &v); info->voltage = v/1000.0f;
    }
    uart_bsp_send_str("AT+CPMUTEMP\r\n");
    if (wait_for_terminal("+CPMUTEMP:", line, sizeof(line), 1000) == ESP_OK) {
        sscanf(line, "+CPMUTEMP: %d", &info->temp);
    }

    UNLOCK_SIM();
    return ESP_OK;
}

esp_err_t sim_a7680c_check_status(void) {
    if (!LOCK_SIM()) return ESP_ERR_TIMEOUT;
    char line[SIM_LINE_BUF_SIZE];

    ESP_LOGI(TAG, "--- DEEP STATUS CHECK ---");

    // 1. PIN Status
    uart_bsp_send_str("AT+CPIN?\r\n");
    if (wait_for_terminal("+CPIN:", line, sizeof(line), 1000) == ESP_OK) {
        ESP_LOGI(TAG, "PIN: %s", line);
    } else {
        ESP_LOGW(TAG, "PIN: Status unknown or error");
    }

    // 2. RF Functionality
    uart_bsp_send_str("AT+CFUN?\r\n");
    if (wait_for_terminal("+CFUN:", line, sizeof(line), 1000) == ESP_OK) {
        ESP_LOGI(TAG, "Radio: %s (1=Full, 0=Min, 4=Flight)", line);
    }

    // 3. Detailed Reg Status
    uart_bsp_send_str("AT+CREG?\r\n");
    if (wait_for_terminal("+CREG:", line, sizeof(line), 1000) == ESP_OK) {
        ESP_LOGI(TAG, "GSM Reg: %s (1=Home, 5=Roam, 0/2=Search/No)", line);
    }

    uart_bsp_send_str("AT+CEREG?\r\n");
    if (wait_for_terminal("+CEREG:", line, sizeof(line), 1000) == ESP_OK) {
        ESP_LOGI(TAG, "LTE Reg: %s", line);
    }

    ESP_LOGI(TAG, "-------------------------");
    UNLOCK_SIM();
    return ESP_OK;
}

esp_err_t sim_a7680c_set_imei(const char* imei) {
    if (!imei || strlen(imei) != 15) return ESP_ERR_INVALID_ARG;
    if (!LOCK_SIM()) return ESP_ERR_TIMEOUT;
    char cmd[32]; snprintf(cmd, sizeof(cmd), "AT+SIMEI=%s\r\n", imei);
    uart_bsp_send_str(cmd);
    esp_err_t err = wait_for_terminal(NULL, NULL, 0, 5000);
    UNLOCK_SIM();
    return err;
}

esp_err_t sim_a7680c_send_sms(const char* phone, const char* message) {
    if (!LOCK_SIM()) return ESP_FAIL;
    uart_bsp_send_str("AT+CMGF=1\r\n");
    wait_for_terminal(NULL, NULL, 0, 1000);
    char cmd[64]; snprintf(cmd, sizeof(cmd), "AT+CMGS=\"%s\"\r\n", phone);
    uart_bsp_send_str(cmd);
    
    // Wait for '>'
    char c; bool got_prompt = false;
    for(int i=0; i<20; i++) {
        if(uart_bsp_read_line(&c, 1, 100) > 0 && c == '>') { got_prompt=true; break; }
    }
    
    esp_err_t err = ESP_FAIL;
    if (got_prompt) {
        uart_bsp_send_str(message);
        const uint8_t ctrl_z = 0x1A; uart_bsp_write(&ctrl_z, 1);
        err = wait_for_terminal("+CMGS:", NULL, 0, 10000);
    }
    UNLOCK_SIM();
    return err;
}

esp_err_t sim_a7680c_make_call(const char* phone) {
    if (!LOCK_SIM()) return ESP_FAIL;
    char cmd[64]; snprintf(cmd, sizeof(cmd), "ATD%s;\r\n", phone);
    uart_bsp_send_str(cmd);
    esp_err_t err = wait_for_terminal(NULL, NULL, 0, 3000);
    UNLOCK_SIM();
    return err;
}

esp_err_t sim_a7680c_hang_up(void) {
    if (!LOCK_SIM()) return ESP_FAIL;
    uart_bsp_send_str("ATH\r\n");
    esp_err_t err = wait_for_terminal(NULL, NULL, 0, 2000);
    UNLOCK_SIM();
    return err;
}

esp_err_t sim_a7680c_ping(void) {
    if (!LOCK_SIM()) return ESP_FAIL;
    esp_err_t err = internal_ping();
    UNLOCK_SIM();
    return err;
}
