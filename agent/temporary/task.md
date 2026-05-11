# Task: Implement OTA Update Feature

## 1. Research & Design
- [x] Verify actual flash size and adjust partition table.
- [x] Design `ota_manager` component architecture.

## 2. Partition Table Update
- [x] Modify `gateway-esp32/partitions.csv` to include `ota_0` and `ota_1`.
- [x] Adjust SPIFFS and App partition sizes to fit 4MB flash.

## 3. OTA Manager Implementation (Middleware)
- [x] Create `gateway-esp32/middleware/connectivity/ota_manager/include/ota_manager.h`.
- [x] Create `gateway-esp32/middleware/connectivity/ota_manager/ota_manager.c`.
- [x] Implement `ota_manager_start(const char *url)` using `esp_https_ota`.
- [x] Implement event callbacks for progress and status reporting.

## 4. Web API Integration
- [x] Update `gateway-esp32/middleware/connectivity/web_server/web_api_handlers.c` to add `/api/ota` POST handler.
- [x] Register the new URI in `web_server.c`.

## 5. UI Implementation
- [x] Update `gateway-esp32/data/index.html` to add OTA update section in Settings.
- [x] Update `gateway-esp32/data/script.js` to handle OTA trigger and show progress.

## 6. Verification
- [x] Build and flash the new firmware with the updated partition table.
- [x] Upload the updated filesystem.
- [ ] Test OTA update using a locally hosted firmware binary.
