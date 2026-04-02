# ST7789 TFT Driver

High-performance driver for the ST7789 240x240 TFT display on the ESMU Gateway (ESP32).

## Features
- **RGB565 Color**: Native 16-bit color support.
- **DMA Acceleration**: Uses SPI DMA for non-blocking high-speed transfers.
- **Geometric Primitives**: Support for lines, rectangles, circles, and fills.
- **Font Engine**: Includes a standard 8x16 font with block-write optimization.
- **Bitmap/Image Support**: APIs for 1-bit monochrome icons and 16-bit RGB565 images.
- **Rotation**: Full 0, 90, 180, 270 degree rotation support.
- **Backlight Control**: Integrated PWM brightness control via `pwm_bsp`.

## Dependencies
- `spi_bsp`: For low-level SPI communication.
- `pwm_bsp`: For backlight brightness.

## Usage
1. Initialize the SPI bus via `spi_bsp_bus_init`.
2. Configure `st7789_config_t` with pins defined in `board_pins.h`.
3. Call `st7789_init()`.
