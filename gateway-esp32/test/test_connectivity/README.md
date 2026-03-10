# Connectivity Stack Unit Tests

This directory contains unit tests for the Elevator Safety Monitoring Unit (ESMU) connectivity layer, covering `wifi_sta`, `mqtt_manager`, and the coordinating `connectivity_manager`.

## What is Tested?

The tests verify the following functionality:

1.  **Initialization (`test_connectivity_init`)**:
    *   Ensures `connectivity_manager_init` correctly processes valid WiFi and MQTT configurations.
2.  **Lifecycle and Status (`test_connectivity_start_and_status`)**:
    *   Verifies that the manager starts its internal orchestrator task successfully.
    *   Checks that the initial state is either `CONNECTIVITY_IDLE` or `CONNECTIVITY_WIFI_ONLY`.
3.  **Error Handling (`test_connectivity_invalid_config`)**:
    *   Verifies that the manager returns `ESP_ERR_INVALID_ARG` when provided with a NULL configuration.

## How to Run the Tests

These tests are designed to run on the target ESP32 hardware.

### Using PlatformIO Core CLI

Open the CLI in your IDE and run:

```bash
pio test -e esp32doit-devkit-v1 -f test_connectivity
```

*Note: Ensure your device is connected to the correct COM port.*

## Implementation Details

*   **Framework**: Unity Test Framework.
*   **Isolation**: Each test case re-initializes the stack. `tearDown()` ensures the connectivity task is stopped and WiFi is disconnected to provide a clean slate for the next test.
*   **Idempotency**: The underlying drivers have been modified to handle multiple initialization calls safely.
