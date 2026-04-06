/**
 * @file web_server.c
 * @brief HTTP Web Server for WiFi Provisioning with URL Decoding
 */

#include "web_server.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "system_registry.h"
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

static const char *TAG = "WEB_SERVER";

static httpd_handle_t server = NULL;

static char s_received_ssid[33] = {0};
static char s_received_pass[64] = {0};
static char s_received_phone[20] = {0};
static bool s_credentials_received = false;

/**
 * @brief Simple URL decoder for form-urlencoded data
 */
static void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

/**
 * @brief Helper to send a file from SPIFFS
 */
static esp_err_t send_file(httpd_req_t *req, const char *path, const char *type) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file : %s", path);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, type);

    char chunk[1024];
    size_t read_bytes;
    do {
        read_bytes = fread(chunk, 1, sizeof(chunk), f);
        if (read_bytes > 0) {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                fclose(f);
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);

    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // Final empty chunk
    return ESP_OK;
}

/* GET / handler */
static esp_err_t get_handler(httpd_req_t *req) {
    return send_file(req, "/spiffs/index.html", "text/html");
}

static const httpd_uri_t get_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = get_handler,
    .user_ctx  = NULL
};

/* GET /style.css handler */
static esp_err_t css_handler(httpd_req_t *req) {
    return send_file(req, "/spiffs/style.css", "text/css");
}

static const httpd_uri_t css_uri = {
    .uri       = "/style.css",
    .method    = HTTP_GET,
    .handler   = css_handler,
    .user_ctx  = NULL
};

/* GET /script.js handler */
static esp_err_t js_handler(httpd_req_t *req) {
    return send_file(req, "/spiffs/script.js", "application/javascript");
}

static const httpd_uri_t js_uri = {
    .uri       = "/script.js",
    .method    = HTTP_GET,
    .handler   = js_handler,
    .user_ctx  = NULL
};

/* POST /config handler */
static esp_err_t post_config_handler(httpd_req_t *req) {
    char buf[256];
    int ret, remaining = req->content_len;

    if (remaining >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Content too long");
        return ESP_FAIL;
    }

    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    /* Parse form-urlencoded data: ssid=NAME&pass=SECRET&phone=NUMBER */
    char *ssid_val = NULL;
    char *pass_val = NULL;
    char *phone_val = NULL;

    char *token = strtok(buf, "&");
    while (token != NULL) {
        if (strncmp(token, "ssid=", 5) == 0) {
            ssid_val = token + 5;
        } else if (strncmp(token, "pass=", 5) == 0) {
            pass_val = token + 5;
        } else if (strncmp(token, "phone=", 6) == 0) {
            phone_val = token + 6;
        }
        token = strtok(NULL, "&");
    }

    if (ssid_val && pass_val && phone_val) {
        url_decode(s_received_ssid, ssid_val);
        url_decode(s_received_pass, pass_val);
        url_decode(s_received_phone, phone_val);
        s_credentials_received = true;

        ESP_LOGI(TAG, "Config decoded -> SSID: [%s], Pass: [%s], Phone: [%s]", 
                 s_received_ssid, s_received_pass, s_received_phone);
        
        httpd_resp_sendstr(req, "<h1>Configuration Received</h1><p>ESMU is connecting to your WiFi...</p>");
        return ESP_OK;
    }

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid form data");
    return ESP_FAIL;
}

static const httpd_uri_t post_uri = {
    .uri       = "/config",
    .method    = HTTP_POST,
    .handler   = post_config_handler,
    .user_ctx  = NULL
};

extern esp_err_t web_api_status_json_handler(httpd_req_t *req);

static const httpd_uri_t status_uri = {
    .uri       = "/api/status",
    .method    = HTTP_GET,
    .handler   = web_api_status_json_handler,
    .user_ctx  = NULL
};

esp_err_t web_server_start(void) {
    if (server != NULL) return ESP_OK;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &get_uri);
        httpd_register_uri_handler(server, &css_uri);
        httpd_register_uri_handler(server, &js_uri);
        httpd_register_uri_handler(server, &post_uri);
        httpd_register_uri_handler(server, &status_uri);
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Error starting server!");
    return ESP_FAIL;
}

esp_err_t web_server_stop(void) {
    if (server != NULL) {
        httpd_stop(server);
        server = NULL;
    }
    return ESP_OK;
}

bool web_server_get_config(char *ssid_out, char *pass_out, char *phone_out) {
    if (s_credentials_received) {
        if (ssid_out) strcpy(ssid_out, s_received_ssid);
        if (pass_out) strcpy(pass_out, s_received_pass);
        if (phone_out) strcpy(phone_out, s_received_phone);
        s_credentials_received = false; // Reset after reading
        return true;
    }
    return false;
}
