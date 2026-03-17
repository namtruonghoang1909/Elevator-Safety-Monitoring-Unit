
// Hardcoded for now - in production, these would be loaded from NVS
#define WIFI_SSID "Bakito Coffee L1"
#define WIFI_PASS "chucngonmieng"

/* Hardware Pins */
#define CONFIG_PIN_PROVISION_BUTTON 15
#define CONFIG_PIN_STATUS_LED      4

/* CAN Hardware Configuration */
#define CAN_TX_PIN           12
#define CAN_RX_PIN           13
#define CAN_BAUD_RATE_KBPS   500

/* I2C Hardware Configuration */
#define I2C_SDA_PIN          21
#define I2C_SCL_PIN          22

/* MQTT related */
#define BROKER_URI "mqtt://app.coreiot.io"
#define CLIENT_ID "ESMU_001"
#define CLIENT_USERNAME "esmu_001"
#define CLIENT_PASSWORD "esmu"

#define MQTT_TOPIC "v1/devices/me/telemetry"