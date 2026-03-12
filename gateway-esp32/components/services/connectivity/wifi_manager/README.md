# WiFi Manager Component

## Overview
The `wifi_manager` is a robust wrapper for the ESP-IDF WiFi driver. it handles the complexity of switching between Station (STA) and Access Point (AP) modes, managing events, and providing connection status telemetry.

## Features
- **Multi-Mode Support**: Easily switch between STA (connecting to a router) and AP (creating a setup hotspot).
- **Auto-Reconnect**: Automatically attempts to reconnect to the AP if the signal is lost.
- **Event Handling**: Centralized event handler for WiFi and IP events (Got IP, Disconnected, etc.).
- **Telemetry**: Provides RSSI (Signal Strength) and IP address data to the system.
- **Security Compatibility**: Configured with a low threshold (`WIFI_AUTH_OPEN`) to ensure compatibility with various router security settings (WPA/WPA2/WPA3).

## Usage
1. **Initialize**: `wifi_manager_init(config)`
2. **Start STA**: `wifi_manager_start_sta()`
3. **Start AP**: `wifi_manager_start_ap(ssid, password)`
4. **Stop**: `wifi_manager_stop()`
5. **Status**: `wifi_manager_is_connected()`
