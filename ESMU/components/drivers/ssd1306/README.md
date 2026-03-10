# SSD1306 OLED Driver

A high-performance I2C driver for the SSD1306 128x64 OLED display, tailored for the ESMU project's visualization needs.

## Features

- **Page-Based Writing**: Efficient memory usage by writing 8-pixel high pages at a time.
- **Hardware Control**: Full support for contrast adjustment, display inversion, and ON/OFF switching.
- **Robustness**: Bounds-handling logic prevents crashes when attempting to draw outside the 128x64 display area.
- **Efficiency**: Optimized for I2C bus bandwidth.

## Bit Mapping Rules

The SSD1306 uses a specific bit-to-pixel mapping:
- **LSB (Bit 0)**: TOP pixel of the page.
- **MSB (Bit 7)**: BOTTOM pixel of the page.
- **Vertical Continuity**: Verified vertical continuity across page boundaries.

## Public API

- `ssd1306_init(cfg, dev_id)`: Initialize the display and assign an I2C device ID.
- `ssd1306_write_page(dev_id, page, data)`: Write 128 bytes to a single page (8 rows).
- `ssd1306_clear(dev_id)`: Clear the entire display.
- `ssd1306_invert_display(dev_id, invert)`: Toggle inverse video mode.
- `ssd1306_set_contrast(dev_id, contrast)`: Set display brightness (0-255).

## Visual Verification

The driver has been verified using a Unity test suite (`test/test_ssd1306`) which confirms:
- Checkerboard pattern continuity.
- Hardware horizontal scrolling.
- Full-screen fill and clear operations.
