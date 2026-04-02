/**
 * @file web_server.c
 * @brief HTTP Web Server for WiFi Provisioning with URL Decoding
 */

#include "web_server.h"
#include "index_html.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <string.h>
#include <ctype.h>

static const char *TAG = "WEB_SERVER";
static httpd_handle_t server = NULL;

static char s_received_ssid[33] = {0};
static char s_received_pass[64] = {0};
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

/* GET / handler */
static esp_err_t get_handler(httpd_req_t *req) {
    httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t get_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = get_handler,
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

    /* Parse form-urlencoded data: ssid=NAME&pass=SECRET */
    char *ssid_val = NULL;
    char *pass_val = NULL;

    char *token = strtok(buf, "&");
    while (token != NULL) {
        if (strncmp(token, "ssid=", 5) == 0) {
            ssid_val = token + 5;
        } else if (strncmp(token, "pass=", 5) == 0) {
            pass_val = token + 5;
        }
        token = strtok(NULL, "&");
    }

    if (ssid_val && pass_val) {
        url_decode(s_received_ssid, ssid_val);
        url_decode(s_received_pass, pass_val);
        s_credentials_received = true;

        ESP_LOGI(TAG, "Credentials decoded -> SSID: [%s], Password: [%s]", s_received_ssid, s_received_pass);
        
        httpd_resp_sendstr(req, "<h1>Configuration Received</h1><p>ESMU is restarting to connect to your WiFi...</p>");
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

esp_err_t web_server_start(void) {
    if (server != NULL) return ESP_OK;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &get_uri);
        httpd_register_uri_handler(server, &post_uri);
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

bool web_server_get_credentials(char *ssid_out, char *pass_out) {
    if (s_credentials_received) {
        if (ssid_out) strcpy(ssid_out, s_received_ssid);
        if (pass_out) strcpy(pass_out, s_received_pass);
        s_credentials_received = false; // Reset after reading
        return true;
    }
    return false;
}
