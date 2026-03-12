# Display Service

The `display_service` provides high-level UI management and real-time visualization for the ESMU system on the SSD1306 OLED display.

## Layered Architecture

The service is organized into three distinct layers for modularity:

| Layer | Responsibility | Key Files |
| :--- | :--- | :--- |
| **Layer 3: Service** | Task management, state-based layout selection, and I2C flushing. | `service.c` |
| **Layer 2: UI** | Complex modular elements (Header, Footer, Monitoring Views). | `src/ui/ui_components.c` |
| **Layer 1: Primitives** | Drawing pixels, lines, ellipses, strings, and bitmaps. | `src/core/primitives.c`, `font.h`, `icons.h` |

## Features

- **Dynamic Refresh**: 15Hz background refresh task.
- **Enhanced Monitoring**: Interactive elevator visualization that tilts (LEFT/RIGHT) and shows motion (UP/DOWN/IDLE).
- **Custom Art Icons**: Includes a realistic 16x8 bumpy cloud bitmap and vertical WiFi signal bars.
- **Provisioning Interface**: Dedicated "CONFIG MODE" view showing "Wi-Fi:" SSID and Web portal IP address.
- **Fault Indicators**: Automatic inverse-video "EMERGENCY" mode triggered by the system state.

## Public API

- `display_service_init()`: Initializes hardware, creates the refresh task, and displays the boot logo.
- `display_service_deinit()`: Stops the task and clears the display.

## UI Layout

- **Header**: Vertical WiFi bars (0-4) and 16x8 realistic MQTT cloud icon.
- **Main View**: 
    - **Monitoring**: Tilting elevator box with motion arrows + Health/Balance text.
    - **Configuring**: Large "CONFIG MODE" title + "Wi-Fi: [SSID]" + "Web: [IP]".
- **Footer**: Shortened uptime (e.g., `UP:01d 02h 30m`), centered project name (`ESMU`), and heartbeat icon.
