# Task: ST7789 Driver Implementation - Phase 1 & 2

## Status
- [x] Update `board_pins.h` with ST7789 pin definitions.
- [x] Create `spi_bsp` component (header and implementation).
- [x] Register `spi_bsp` in the build system.
- [x] Create `pwm_bsp` for backlight control.
- [x] Implement core `st7789` driver (init, primitives, fonts, bitmaps).
- [x] Verify driver with exhaustive test in `main.c`.
- [x] Clean up `main.c` for clean component-only commit.
- [x] Document new components in READMEs.

## Details
- Driver is ready for high-level `display_service` integration.
- Hardware verified via isolated main test (reverted).
