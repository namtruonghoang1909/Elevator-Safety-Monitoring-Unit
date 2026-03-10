# TODO: Display Service & Debugging Improvements

## Phase 3: UI Components (COMPLETE)
- [x] Design and implement Status Bar (WiFi/MQTT icons).
- [x] **NEW**: Enhanced Monitoring View
    - [x] Implement `ui_draw_elevator_box(x, y, tilt_angle)` helper.
    - [x] Draw elevator on Left with arrows inside.
    - [x] Improve `ui_draw_monitoring_view` layout and balance string handling.
    - [x] Update `motion_task.c` to send descriptive balance strings (LEFT/RIGHT).
- [x] Implement Fault Overlay (Inverse video alert).
- [x] Implement Boot Logo (ESMU in Oval).
- [x] UI Adjustments: Move MQTT icon to header left, center "ESMU" in footer.

## Phase 5: Debugging & Standards Compliance (COMPLETE)
- [x] Add `esp_err_t` return value checks and logging across all components.
    - [x] Update `system_controller.c`
    - [x] Update `wifi_sta.c`
    - [x] Update `display_service.c` (now `service.c`)
    - [x] Update `connectivity_manager.c`
    - [x] Update `mqtt_manager.c`
- [x] Replace `ESP_ERROR_CHECK` with explicit logging where graceful recovery is possible.

## Phase 6: Maintenance & Documentation (COMPLETE)
- [x] Simplify internal file naming (remove `display_` and `motion_` prefixes).
- [x] Update all project READMEs and create missing component documentation.
