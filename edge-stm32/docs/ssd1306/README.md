# SSD1306 Driver & Graphics Rules

This document defines the physical and logical rules for writing data to the SSD1306 OLED display (128x64) in this project. **Always refer to these rules before implementing UI or graphics logic.**

---

## 1. Physical Layout
- **Resolution**: 128 (Width) × 64 (Height) pixels.
- **Pages**: The display is divided into **8 Pages** (Page 0 to Page 7).
- **Page Height**: Each page is exactly **8 pixels high**.
- **Page 0**: Top of the screen (Rows 0-7).
- **Page 7**: Bottom of the screen (Rows 56-63).

---

## 2. Bit-to-Pixel Mapping (Vertical Orientation)
The SSD1306 uses a **Vertical Byte** structure. When you write a single byte to the display, it fills 8 vertical pixels in the current column.

**CRITICAL RULE: LSB-TOP / MSB-BOTTOM**
- **Bit 0 (0x01)**: Maps to the **TOP-most** pixel of the page.
- **Bit 7 (0x80)**: Maps to the **BOTTOM-most** pixel of the page.

### Visual Representation of 1 Byte in a Page:
```text
Bit 0 (LSB) -> [ Pixel ] (Row N)
Bit 1       -> [ Pixel ]
Bit 2       -> [ Pixel ]
Bit 3       -> [ Pixel ]
Bit 4       -> [ Pixel ]
Bit 5       -> [ Pixel ]
Bit 6       -> [ Pixel ]
Bit 7 (MSB) -> [ Pixel ] (Row N+7)
```

---

## 3. Communication Protocol (I2C)
All transactions must start with a **Control Byte**:
- **Command Mode**: `0x00` (Following bytes are interpreted as setup commands).
- **Data Mode**: `0x40` (Following bytes are stored in GDDRAM as pixels).

---

## 4. Addressing Mode
This driver is configured for **Horizontal Addressing Mode** (`0x20, 0x00`):
1. After writing a byte, the **Column Pointer** automatically increments.
2. When the Column Pointer reaches 127, it wraps back to 0 and the **Page Pointer** increments.
3. This allows for a single "Burst Write" of the entire 1024-byte framebuffer.

---

## 5. How to Write Properly (Step-by-Step)

### A. To set a specific pixel (X, Y):
1. Calculate Page: `page = Y / 8`
2. Calculate Bit: `bit = Y % 8`
3. In the framebuffer: `buffer[page * 128 + X] |= (1 << bit)`

### B. To draw a horizontal line:
- Since bits are vertical, a horizontal line across 128 pixels requires writing the **same bit** across 128 bytes in a specific page.
- *Example (Top-most line of screen)*: Write `0x01` 128 times to Page 0.

### C. To draw a vertical line:
- This involves writing `0xFF` to multiple pages at the same X coordinate.

---

## 6. Performance Guidelines
- **Burst Writes**: Always prefer writing a full page or the full screen in one I2C transaction (`i2c_write_bytes`) rather than individual bytes.
- **Dirty Tracking**: Only update pages that have changed to save I2C bandwidth.
