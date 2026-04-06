# Web Server Component

## Overview
The `web_server` component provides a robust HTTP server for **WiFi Provisioning** and **Real-time System Diagnostics**. It transforms the device into a management portal when in Access Point (AP) mode or accessible via the local network.

## Features
- **Modern Diagnostic Dashboard**: A professional "Dark Mode" Single Page Application (SPA) for live telemetry.
- **Real-time Monitoring**: Polling-based UI updates for vibration, motion state, network health, and system status.
- **SoftAP Provisioning**: Integrated configuration portal for WiFi credentials and Emergency Contact details.
- **JSON API**: Provides a RESTful `/api/status` endpoint for external monitoring and UI data binding.
- **SPIFFS Integration**: Serves assets (`index.html`, `style.css`, `script.js`) directly from the flash filesystem, decoupling UI from firmware logic.
- **URL Decoding**: Robust handling of form-urlencoded data including special characters and spaces.

## Architecture
- `web_server.c`: High-level URI routing and server lifecycle management.
- `web_api_handlers.c`: REST API logic and `cJSON`-based serialization of the `system_registry`.
- `web_data/`: Source folder for frontend assets (packed into SPIFFS image).

## API Endpoints
- `GET /`: Serves the main SPA Dashboard/Settings portal.
- `GET /api/status`: Returns a JSON snapshot of the `system_status_registry_t`.
- `POST /config`: Receives and decodes WiFi and Emergency Phone configuration.

## Usage
1. **Start Server**: `web_server_start()`
2. **Retrieve Configuration**: `web_server_get_config(ssid, pass, phone)`
3. **Stop Server**: `web_server_stop()`

## Frontend Development
Web assets are located in the `gateway-esp32/data/` folder (mirrored from `web_data/` for build compatibility). After modifying HTML/CSS/JS, run:
```bash
pio run --target uploadfs
```
This will repack the SPIFFS image and upload it to the ESP32.
