# SSD1306 Driver Verification Suite

This test suite verifies the functionality and robustness of the SSD1306 OLED driver using the Unity framework.

## Test Configuration
- **SDA**: GPIO 21 (Default I2C)
- **SCL**: GPIO 22 (Default I2C)
- **Bus ID**: 0

## Automated Tests
- `test_ssd1306_init_success`: Verifies initial communication and hardware identity.
- `test_ssd1306_commands`: Tests contrast, inverse, and flipping commands.
- `test_ssd1306_bounds`: Ensures that drawing outside the 128x64 area doesn't cause crashes.

## Interactive Tests (Visual Verification)
These tests require visual confirmation. The **Status LED (GPIO 15)** will blink during the "waiting" period of each test.

### 1. Bit Markers (`test_ssd1306_visual_bit_markers`)
- **What you will see**: Two distinct horizontal lines.
    - Top: A single-pixel line at the absolute top (Row 0).
    - Bottom: A single-pixel line at the absolute bottom (Row 63).
- **Verification**: Confirms that Bit 0 maps to the Top pixel and Bit 7 maps to the Bottom pixel of a page.

### 2. Staircase (`test_ssd1306_visual_staircase`)
- **What you will see**: A stepped pattern descending from left to right.
- **Verification**: Confirms that increasing bit significance (Bit 0 -> Bit 7) moves the pixel physically **DOWNWARDS**.

### 3. Checkerboard Pattern (`test_ssd1306_visual_checkerboard`)
- **What you will see**: A fine grid of alternating black and white pixels across the entire 128x64 area.
- **Verification**: Ensure there are no "dead" (always black) or "stuck" (always white) pixels. The pattern should be perfectly uniform.
