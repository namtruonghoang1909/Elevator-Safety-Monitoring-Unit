# UART BSP (Broadband System Peripheral)

## Overview
This component provides a thread-safe UART abstraction for communicating with SIM modules (SIM7600/A7680C).

## Features
- **Thread-safe TX**: Mutex-protected writing.
- **Background URC Classification**: Automatically separates Unsolicited Result Codes (URCs) from standard command responses.
- **Asynchronous Processing**: Uses a background task to buffer incoming lines.
- **Response Queue**: Decouples AT command execution from UART RX timing.

## Usage
1. Initialize the BSP with `uart_bsp_init`.
2. Send a command with `uart_bsp_send_str("AT\r\n")`.
3. Read the response with `uart_bsp_read_response`.

```c
uart_bsp_config_t cfg = {
    .port = UART_NUM_2,
    .baud_rate = 115200,
    .tx_io_num = 17,
    .rx_io_num = 16,
    .urc_cb = my_urc_handler
};
uart_bsp_init(&cfg);
```
