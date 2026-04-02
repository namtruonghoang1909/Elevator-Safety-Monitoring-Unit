# SIM A7680C Driver

High-level driver for the SIM7600/A7680C/A7670C series LTE modules, optimized for the ESMU Gateway.

## Features
- **Hardware Power Control**: Robust reset/power-on sequence using GPIO 5 (PWRKEY).
- **Network Status**: Monitoring of signal quality (CSQ), registration status (CREG), and operator info.
- **SMS Support**: Text-mode SMS sending with prompt handling.
- **Voice Calls**: Dialing and hang-up support for emergency alerts.
- **Thread-Safe**: Designed to work with the ESMU UART BSP's mutex-protected reads/writes.

## API Usage

### Initialization
```c
sim_a7680c_hw_reset(); // Pulse PWRKEY
sim_a7680c_init();     // Sync baud and disable echo
```

### SMS
```c
sim_a7680c_send_sms("+84123456789", "Emergency: Elevator Shake Detected!");
```

### Voice Call
```c
sim_a7680c_make_call("+84123456789");
vTaskDelay(pdMS_TO_TICKS(15000));
sim_a7680c_hang_up();
```

## Hardware Requirements
- **UART**: Connected to ESP32 UART2 (TX 17, RX 16).
- **Baud Rate**: 9600 bps (standardized for stability).
- **Power**: High-current 3.8V-4.2V supply (capable of 2A peaks).
