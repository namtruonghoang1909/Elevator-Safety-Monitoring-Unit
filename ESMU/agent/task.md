# task 01

create a todo list to implement, only implement before i approve the todo list:
stores the todo list into the todo.md in the agent folder

connectivity folder contains
- wifi_sta component. 
    + wifi_sta_init will initialize by taking in a user config_t
    + connect, disconnect APIs
    + wifi connection state get APIs
    + auto reconnects will be enanle or disable by the config type 

the purpose of this component is to wraps the esp wifi APIs inorder for high level code to use

- take a look at:
https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_wifi.html
and

after implementation, implement main.c to test the wifi_sta
connects to 
ssid : Bakito Coffee L1
pw: chucngonmieng

# task 02
implements a public function in the that creates a task that logs wifi health every 5 seconds with this format
---- wifi health ----
ssid: ....
state: connected or not
strength:  strong, weak, ...

# task 03
implements the mqtt_manager as a wrapper for esp_mqtt_client.
provide APIs for:
- init with config (uri, client_id, etc.)
- start/stop
- publish/subscribe
- connection status check

integrate into main.c to test publishing when wifi is connected.
# task 04
implements the connectivity_manager to coordinate wifi_sta and mqtt_manager.
provide APIs for:
- init with combined config
- start
- is_ready (WiFi + MQTT)
- get_state

refactor main.c to use this manager instead of raw calls.

# task 05
CONNECTIVITY TEST
- implement test for connectivity with these credentials:
    - wifi:
        ssid = "Bakito Coffee L1"
        password = "chucngonmieng"
        auto_reconnect = true
    - mqtt:
        broker_uri = "mqtt://app.coreiot.io",
        client_id = "ESMU_001",
        username = "esmu_001",
        password = "esmu",
        port=1883,
    - publish to a test topic to ("v1/devices/me/telemetry") on broker

- add a short readme to provide a overview for the test and instructions

--------------------