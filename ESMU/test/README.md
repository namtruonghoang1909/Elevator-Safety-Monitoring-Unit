# ESMU Testing Framework

This directory contains automated tests for the Elevator Safety Monitoring Unit (ESMU) project, implemented using the Unity test framework.

## Test Structure

- `test_connectivity/`: Unit tests for the connectivity stack, including `wifi_sta`, `mqtt_manager`, and `connectivity_manager`.

## How to Run Tests

Open the **PlatformIO Core CLI** in your IDE and run the following commands:

### Run all tests:
```bash
pio test -e esp32doit-devkit-v1
```

### Run a specific test module (e.g., connectivity):
```bash
pio test -e esp32doit-devkit-v1 -f test_connectivity
```

## Adding New Tests

1. Create a new directory under `test/` (e.g., `test_sensors`).
2. Create a C file inside (e.g., `test_sensors.c`).
3. Include `<unity.h>` and implement `setUp()`, `tearDown()`, and `app_main()`.
4. Define your test functions and run them using `RUN_TEST()`.

## Important Notes

- Tests require NVS initialization, which is handled in `setUp()` for connectivity tests.
- Ensure the WiFi credentials in the test files are correct for your environment.
