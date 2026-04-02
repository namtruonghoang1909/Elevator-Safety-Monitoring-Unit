# Connectivity Stack

The connectivity stack manages the ESMU system's external communications, including WiFi provisioning, MQTT telemetry, and future cellular/SMS capabilities.

## Components

### 1. WiFi Station (`wifi_sta`)
- Manages connection to local Access Points.
- Implements auto-reconnect logic.
- Provides RSSI monitoring and IP retrieval.

### 2. MQTT Manager (`mqtt_manager`)
- High-level wrapper for the ESP-MQTT client.
- Handles connection lifecycle and event dispatching.
- Provides thread-safe publish/subscribe interfaces.

### 3. Connectivity Manager (`connectivity_manager`)
- **Orchestrator**: Manages the combined lifecycle of WiFi and MQTT.
- **Stability Monitoring**: Waits for stable WiFi (5 seconds) before initiating MQTT.
- **System Integration**: Updates the `system_registry` with real-time connectivity status (0-4 signal bars, connection flags).

## Architecture

The `connectivity_manager` acts as the single point of entry for the application. It internally initializes `wifi_sta` and `mqtt_manager` based on a unified configuration structure.

## Configuration

Connectivity is configured via `connectivity_config_t`, which includes:
- SSID and Password.
- MQTT Broker URI and Client Credentials.
- Auto-reconnect toggles.

## Status Mapping

RSSI values are mapped to visual "bars" for the display:
- `> -55 dBm`: 4 bars
- `> -65 dBm`: 3 bars
- `> -75 dBm`: 2 bars
- `> -85 dBm`: 1 bar
- `Otherwise`: 0 bars
