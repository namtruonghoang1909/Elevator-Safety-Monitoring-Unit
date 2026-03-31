# ESMU Gateway Services

This directory contains the high-level services of the ESMU Gateway Node. Each service runs in its own FreeRTOS task and communicates via the System Registry and event system.

## 🚀 Services

### 📡 `communication/`
Manages the inter-node and cloud communication protocols.
- **CAN Manager**: Handles TWAI/CAN bus reception and message decoding using the shared ESMU protocol.
- **MQTT Proxy**: Translates internal registry updates into MQTT telemetry payloads for cloud ingestion.

### 🌐 `connectivity/`
Orchestrates the lifecycle of networking interfaces.
- **WiFi Manager**: Handles station connection, auto-reconnect, and SoftAP provisioning.
- **SIM7600 Manager**: (In Development) Provides cellular fallback for critical alerts.

### 🎮 `motion_proxy/`
Acts as a data broker for sensor information received from the Edge node.
- **Data Aggregator**: Receives filtered IMU metrics via CAN and updates the local system registry.
- **Health Analyzer**: Monitors heartbeats from the Edge node to ensure sensor availability.

### 📺 `display/`
Manages the local user interface.
- **UI Controller**: Renders real-time dashboards on the ST7789/SSD1306 displays.
- **Diagnostic Logger**: Provides a "Worst-Case" log view for field troubleshooting.

---
**Part of the Elevator Safety Monitoring Unit (ESMU).**
