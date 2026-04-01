
# Code Plan: ST7789 Driver Implementation (ESP32 Gateway Node)

## 1. Objective
Implement a high-performance ST7789 240x240 TFT display driver using SPI with DMA support on the ESP32 Gateway node. This will replace the SSD1306 as the primary status display.

## 2. Implementation Steps (TODO)

### Phase 1: SPI BSP & Pin Configuration
- [ ] Update `gateway-esp32/components/bsp/board_pins/include/board_pins.h` with ST7789 pins.
    - `ST7789_SCL`: 18
    - `ST7789_SDA`: 23
    - `ST7789_CS`: 26
    - `ST7789_DC`: 27
    - `ST7789_RST`: 33
    - `ST7789_BL`: 32
- [ ] Create `gateway-esp32/components/bsp/spi_bsp/`.
    - [ ] `include/spi_bsp.h`: Define SPI initialization and device management APIs.
    - [ ] `spi_bsp.c`: Implement SPI bus initialization using VSPI and DMA.
- [ ] Create `gateway-esp32/components/bsp/pwm_bsp/` (optional but recommended for backlight).
    - [ ] Implement simple LEDC wrapper for backlight brightness control.

### Phase 2: ST7789 Core Driver
- [ ] Create `gateway-esp32/components/drivers/st7789/`.
    - [ ] `include/st7789.h`: Define display configuration and drawing APIs.
    - [ ] `st7789.c`:
        - [ ] Implement hardware reset and initialization sequence.
        - [ ] Implement `st7789_set_window()` to define drawing areas.
        - [ ] Implement `st7789_push_colors()` using SPI DMA for high-speed transfers.
        - [ ] Implement `st7789_fill_screen()` and basic pixel/rectangle drawing.

### Phase 3: Display Service Readiness (Preparation)
- [ ] Implement a simple color test pattern to verify driver integrity.
- [ ] Verify SPI frequency (target 40MHz) and DMA stability.
- [ ] Implement "Partial Buffer" support to allow drawing without a full 115KB framebuffer.

## 3. Verification & Testing
- [ ] **Hardware Verification**: Verify all pins are correctly connected and there are no conflicts with SIM7600 or CAN.
- [ ] **Initialization Test**: Verify the screen turns on and clear correctly (black).
- [ ] **Performance Test**: Measure time to fill the screen (target < 20ms at 40MHz).
- [ ] **Stability Test**: Run test patterns for 1 hour to ensure DMA doesn't hang.

## 4. Dependencies
- ESP-IDF `driver/spi_master`
- ESP-IDF `driver/ledc` (for backlight)
- `board_pins.h` for GPIO definitions
