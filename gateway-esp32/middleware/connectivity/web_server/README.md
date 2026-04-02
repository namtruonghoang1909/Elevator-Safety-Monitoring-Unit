# Web Server Component

## Overview
The `web_server` component provides a lightweight HTTP server for **WiFi Provisioning**. It allows users to configure the device's WiFi credentials via a web browser when the device is in Access Point (AP) mode.

## Features
- **SoftAP Portal**: Serves a mobile-friendly HTML form at `http://192.168.4.1`.
- **URL Decoding**: Robust handling of special characters and spaces (e.g., `+` converted to ` `) in SSIDs and Passwords.
- **Asynchronous Flow**: Stores received credentials in an internal buffer for the `connectivity_manager` to retrieve.
- **Embedded UI**: HTML content is declared in `include/index_html.h` for clean separation from C logic.

## Usage
1. **Start Server**: `web_server_start()`
2. **Retrieve Credentials**: `web_server_get_credentials(ssid, pass)` returns `true` if new credentials have been submitted.
3. **Stop Server**: `web_server_stop()`

## UI Customization
To modify the provisioning page, update the `INDEX_HTML` string in `include/index_html.h`.
