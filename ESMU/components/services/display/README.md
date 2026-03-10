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
- **Status Dashboard**: Integrated header with WiFi/MQTT icons and a footer with uptime and system heartbeats.
- **Fault Indicators**: Automatic inverse-video "EMERGENCY" mode triggered by the system state.
- **Resource Optimized**: Shortened strings and efficient bitmap drawing to minimize I2C overhead.

## Public API

- `display_service_init()`: Initializes hardware, creates the refresh task, and displays the boot logo.
- `display_service_deinit()`: Stops the task and clears the display.

## UI Layout

- **Header**: WiFi signal bars (0-4) and MQTT cloud icon.
- **Main View**: 
    - Left: Tilting elevator box with motion arrows.
    - Right: Health status and Balance description.
- **Footer**: Shortened uptime (e.g., `U:01d02h30m`), centered project name (`ESMU`), and heartbeat icon.
