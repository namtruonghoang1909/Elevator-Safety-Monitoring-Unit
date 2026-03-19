# CAN Bus Playground Test Plan (Direct in main.c)

## Objective
Verify CAN communication between STM32 and ESP32 by implementing simple transmission and reception logic directly in the `main.c` files of both nodes. This validates the hardware and BSP layers before building higher-level services.

---

## 1. STM32 (Edge Node) - "The Transmitter"

### Goal: Send dummy ESMU protocol packets directly from the default FreeRTOS task.

- **Timing Configuration**:
  - Update `MX_CAN_Init` in `main.c` for 500kbps (Prescaler: 1, BS1: 12, BS2: 3).
- **Initialization**:
  - Include `bsp_can.h` and `esmu_protocol.h`.
  - Call `bsp_can_init()` in `main()` before `osKernelStart()`.
- **Test Logic (in `StartDefaultTask`)**:
  - Every 1000ms: Send `edge_heartbeat_t` (ID 0x200).
  - Every 100ms: Send `ele_health_t` (ID 0x100) with dummy tilt values.
  - Use `bsp_can_send()` for transmission.

---

## 2. ESP32 (Gateway Node) - "The Receiver"

### Goal: Enable the TWAI driver and Motion Proxy to bridge real CAN traffic to the system.

- **Main Refactor (`app_main`)**:
  - Include `can_bsp.h` and `motion_proxy/src/core/task.h`.
  - Initialize `can_bsp` (Pins 12/13, 500kbps) and `can_bsp_start()`.
  - Call `mp_task_init()` to start the background listener.
  - **Comment out** `mock_data_provider_task` creation.
- **Verification**:
  - Monitor serial output for "CAN Proxy listener" logs.
  - Verify SSD1306 Display shows the dummy data sent from STM32.

---

## 3. Hardware Requirements
- CAN Transceivers connected to both nodes.
- 120-ohm termination resistors.
- Common ground.

---

## 4. Success Criteria
- [ ] ESP32 serial logs show reception of IDs 0x100 and 0x200.
- [ ] SSD1306 displays "RUNNING" (from STM32 heartbeat).
- [ ] No "COMMUNICATION LOST" message on ESP32 OLED.
