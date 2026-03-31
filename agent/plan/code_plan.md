

# Code Plan: SIM A7680C Integration (ESP32 Gateway Node)

## 1. Objective
Implement a robust UART-based driver for the SIM A7680C module to enable LTE connectivity and SMS alerts.

## 2. Implementation Steps (TODO)

### Phase 1: UART BSP
- [ ] Create `gateway-esp32/components/bsp/uart_bsp/include/uart_bsp.h`.
- [ ] Implement `gateway-esp32/components/bsp/uart_bsp/uart_bsp.c`.
    - [ ] Initialize UART2 (default for SIM module).
    - [ ] Implement thread-safe `uart_bsp_write()` and `uart_bsp_read_line()`.
    - [ ] Implement background task for URC monitoring (Unsolicited Result Codes).

### Phase 2: A7680C Core Driver
- [ ] Create `gateway-esp32/components/drivers/a7680c/`.
- [ ] Implement command/response parser with timeout handling.
- [ ] Implement module lifecycle management:
    - [ ] `a7680c_hw_power_on()` (Pulse PWR_KEY).
    - [ ] `a7680c_init()` (Sync baud rate, disable echo).
    - [ ] `a7680c_check_status()` (SIM card, Network registration).

### Phase 3: Telemetry & SMS Services
- [ ] Implement SMS API: `a7680c_send_sms(const char* phone, const char* msg)`.
- [ ] Implement LTE-based MQTT (using module's internal stack):
    - [ ] `a7680c_mqtt_connect()`, `a7680c_mqtt_publish()`.

### Phase 4: Connectivity Manager Integration
- [ ] Refactor `connectivity_manager.c` to support "Connectivity Priority" (WiFi > LTE).
- [ ] Implement failover logic: If `WIFI_DISCONNECTED`, attempt `LTE_ATTACH`.
- [ ] Update `system_registry` to include LTE signal bars and carrier name.

## 3. Verification & Testing
- [ ] **Unit Test (UART)**: Loopback TX/RX test.
- [ ] **AT Command Test**: Send `AT`, expect `OK`.
- [ ] **SMS Test**: Verify SMS received on a physical phone during simulated fault.
- [ ] **Failover Test**: Turn off WiFi and verify MQTT telemetry continues via LTE.
