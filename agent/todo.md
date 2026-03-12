# TODO: Distributed System & Hardware Integration

## Phase 7: WiFi Provisioning System (COMPLETE)
- [x] Implement NVS Helper for credential storage.
- [x] Refactor `wifi_sta` to a multi-mode `wifi_manager`.
- [x] Implement `web_server` component with provisioning form.
    - [x] Add **URL Decoding** for spaces and special characters.
    - [x] Separate UI into `index_html.h`.
- [x] Integrate provisioning logic into `connectivity_manager`.
    - [x] Implement **Bidirectional Toggle**: 5s hold on GPIO 15 switches between STA and AP.
    - [x] Implement **Status LED blinking** (GPIO 4).
- [x] Update `display_service` to show "CONFIG MODE" with "Wi-Fi:" and Web IP.
- [x] Redesign Status Bar icons (**Windows-style WiFi** and **Realistic Cloud**).

## Phase 8: Distributed System & Fault Detection
- [ ] Define `shared/protocol/esmu_protocol.h`.
- [ ] Port MPU6050 driver to STM32.
- [ ] Implement `fault_detector` logic on STM32.
- [ ] Implement CAN communication on both Gateway (ESP32) and Edge (STM32).
- [ ] Update Gateway UI to display remote telemetry.

## Phase 9: System Hardening
- [ ] Implement watchdog timers for all major tasks.
- [ ] Add MQTT command handling for remote configuration.
- [ ] Implement OTA (Over-the-Air) updates via Web Server.
