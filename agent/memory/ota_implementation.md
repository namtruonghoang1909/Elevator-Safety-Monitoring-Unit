# Project Memory - OTA Update Feature Implementation

## [2026-05-11] OTA Update Feature Implementation
- **Feature**: HTTPS-based Over-The-Air (OTA) firmware updates.
- **Component**: `ota_manager` (middleware/connectivity).
- **Changes**:
    - Updated `partitions.csv` to support dual OTA partitions (`ota_0`, `ota_1`) and resized SPIFFS to 768KB.
    - Implemented `ota_manager` middleware using `esp_https_ota`.
    - Added `/api/ota` POST endpoint and updated `/api/status` to report progress in `web_api_handlers.c`.
    - Integrated OTA UI in the web dashboard (HTML/JS/CSS).
    - Initialized `ota_manager` in `app_main`.
- **Note**: Certificate bundle support (`esp_crt_bundle`) was disabled due to build environment resolution issues. OTA currently works over plain HTTP or HTTPS without certificate validation.
