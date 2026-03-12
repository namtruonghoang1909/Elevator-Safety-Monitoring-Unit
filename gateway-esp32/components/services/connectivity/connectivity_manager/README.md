# Connectivity Manager Component

## Overview
The `connectivity_manager` is the central orchestrator for all network-related activities. It manages the lifecycle of WiFi and MQTT, ensuring the device stays connected and handles provisioning when needed.

## Features
- **WiFi Orchestration**: Automatically connects to stored credentials or triggers provisioning mode.
- **MQTT Lifecycle**: Manages MQTT client start/stop based on WiFi stability.
- **Hardware Integration**:
    - **Provisioning Button (GPIO 15)**: Hold for 5 seconds to manually enter Config Mode.
    - **Status LED (GPIO 4)**: Blinks rapidly during provisioning; stays off or reflects state otherwise.
- **NVS Integration**: Persistently stores and retrieves WiFi credentials.
- **Display Updates**: Updates the `system_registry` with connection status and provisioning details for the UI.

## Provisioning Workflow
1. At boot, if no credentials exist, the manager enters `CONNECTIVITY_PROVISIONING`.
2. It starts the `wifi_manager` in AP Mode (`ESMU-Setup`) and launches the `web_server`.
3. The Status LED (GPIO 4) blinks at 100ms intervals.
4. Once credentials are received via the web portal, they are saved to NVS.
5. The system restarts the WiFi in Station mode to connect to the new network.

## Manual Trigger
Users can force the device into Provisioning mode at any time by pressing and holding the button on **GPIO 15 for 5 seconds**.
