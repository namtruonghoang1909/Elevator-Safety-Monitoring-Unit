#include <string.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt_manager.h"

static const char *TAG = "MQTT_MGR";

static esp_mqtt_client_handle_t s_client = NULL;
static bool s_is_connected = false;

/**
 * @brief MQTT Event handler function
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED to broker");
        s_is_connected = true;
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED from broker");
        s_is_connected = false;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA received");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGE(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGE(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", (int)event_id);
        break;
    }
}

esp_err_t mqtt_manager_init(const mqtt_manager_config_t *config)
{
    if (config == NULL || config->broker_uri == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = config->broker_uri,
        .broker.address.port = config->port,
        .credentials.client_id = config->client_id,
        .credentials.username = config->username,
        .credentials.authentication.password = config->password,
        .session.keepalive = config->keepalive > 0 ? config->keepalive : 60,
        .session.disable_clean_session = false,
        .network.disable_auto_reconnect = config->disable_auto_reconnect
    };

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client handle");
        return ESP_FAIL;
    }

    // Register events
    esp_err_t err = esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register MQTT events: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "MQTT manager initialized for broker: %s", config->broker_uri);
    return ESP_OK;
}

esp_err_t mqtt_manager_start(void)
{
    if (s_client == NULL) return ESP_ERR_INVALID_STATE;
    ESP_LOGI(TAG, "Starting MQTT client...");
    return esp_mqtt_client_start(s_client);
}

esp_err_t mqtt_manager_stop(void)
{
    if (s_client == NULL) return ESP_ERR_INVALID_STATE;
    ESP_LOGI(TAG, "Stopping MQTT client...");
    return esp_mqtt_client_stop(s_client);
}

esp_err_t mqtt_manager_publish(const char *topic, const char *data, int qos, bool retain)
{
    if (s_client == NULL || !s_is_connected) {
        ESP_LOGW(TAG, "Cannot publish: Client not connected");
        return ESP_ERR_INVALID_STATE;
    }

    int msg_id = esp_mqtt_client_publish(s_client, topic, data, 0, qos, retain);
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to publish message (msg_id=%d)", msg_id);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Sent publish successful, msg_id=%d", msg_id);
    return ESP_OK;
}

esp_err_t mqtt_manager_subscribe(const char *topic, int qos)
{
    if (s_client == NULL || !s_is_connected) {
        ESP_LOGW(TAG, "Cannot subscribe: Client not connected");
        return ESP_ERR_INVALID_STATE;
    }

    int msg_id = esp_mqtt_client_subscribe(s_client, topic, qos);
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to subscribe (msg_id=%d)", msg_id);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sent subscribe successful, msg_id=%d", msg_id);
    return ESP_OK;
}

esp_err_t mqtt_manager_unsubscribe(const char *topic)
{
    if (s_client == NULL || !s_is_connected) return ESP_ERR_INVALID_STATE;
    
    int msg_id = esp_mqtt_client_unsubscribe(s_client, topic);
    if (msg_id < 0) return ESP_FAIL;
    
    return ESP_OK;
}

bool mqtt_manager_is_connected(void)
{
    return s_is_connected;
}
