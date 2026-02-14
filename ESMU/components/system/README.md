# SYSTEM HARDWARES COMPONENTS
    - Signal leds
        + led for gsm (red when no configuration, green for configuration provided)
        + led for mqtt (red when no connection, green for connected to broker)
        + led for elevator health  (green when elevator behaves normally, yellow for degreaded(vibrates slightly, small shakes, ....), red for critical faults)



# SYSTEM FLOW

- the system boots up, then calls system_start();
    - system start will post SYSTEM_EVENT_BOOT.
- The system will call device_init()
    - intializes:
        - hardware: 
            - pins(gpio, i2c, ...)
            - sensors(imu, ...)
        - software: 
            - nvs 
            - event loop
    
    - starts wifi connection and mqtt , sms in sequence (which calls wifi_init, wifi_init waits on configuration event bits)
        - if connected to broker, green led for wifi on. Otherwise red.

- the system will go to monitoring, ready to receive fault events.

- when user access the webserver config, the system will receive SYSTEM_EVENT_START_CONFIGURATION
    - throughout the system operation,
        - if the sms infos arent provided, red led for gsm blinks.
        - if the wifi infos arent provided, the system still operates normally.
- after user save the config, system will receive SYSTEM_EVENT_CONFIGURATION_DONE, set the configuration event bits



- when system receives SYSTEM_EVENT_ELEVATOR_FAULT_DETECTED, the system attempts to send call and sms, mqtt on emergency topic.
    - sms only sends when phone infos is provided by the configuration
    - mqtt only connected to broker
