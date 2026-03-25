# Code Plan: ESMU System Refactor & ST7789 Upgrade

## I. Gateway ESP32 System Refactor (Functional Reorganization) [COMPLETED]
Reorganize the `system` component into a functional folder structure where both header (`.h`) and source (`.c`) files are co-located.

### Phase 1: Directory Setup [x]
### Phase 2: File Migration [x]
### Phase 3: Build Configuration Update [x]
### Phase 4: Code Update & Verification [x]

---

## II. SPI BSP (Board Support Package) Implementation
Create a thread-safe SPI master abstraction layer for ESP32.

### Tasks:
- [ ] Create `gateway-esp32/components/bsp/spi_bsp/include/spi_bsp.h`.
    - Define handle-based IDs for SPI buses and devices.
    - API for bus initialization and device addition.
    - API for command/data transfers (ST7789 needs DC pin control).
- [ ] Create `gateway-esp32/components/bsp/spi_bsp/spi_bsp.c`.
    - Implement ESP-IDF SPI Master driver wrapper.
    - Handle GPIO configurations for DC, RST pins if needed via abstraction.
- [ ] Create `gateway-esp32/components/bsp/spi_bsp/CMakeLists.txt`.

---

## III. ST7789 SPI Driver Implementation
Implement the driver for the 240x240 (or similar) color TFT display.

### Tasks:
- [ ] Create `gateway-esp32/components/drivers/st7789/include/st7789.h`.
- [ ] Create `gateway-esp32/components/drivers/st7789/st7789.c`.
    - Implement initialization sequence (SW reset, Wake up, Color mode, Pixel format).
    - Implement basic primitives (fill_screen, draw_pixel, draw_rect).
    - Implement framebuffer or partial window updates for efficiency.
- [ ] Create `gateway-esp32/components/drivers/st7789/CMakeLists.txt`.

---

## IV. Display Service Migration
Update the `display_service` to support ST7789 and potentially handle color UI.

### Tasks:
- [ ] Refactor `display_service.c` to use an abstraction for different display types (SSD1306 vs ST7789).
- [ ] Implement a basic "Dashboard" UI for ST7789 showing CAN metrics and node status.
- [ ] Verify both SSD1306 and ST7789 functionality.

---
*Plan Updated: March 22, 2026*
