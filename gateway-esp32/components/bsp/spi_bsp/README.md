# SPI BSP

Board Support Package for SPI Master management on ESP32.

## Features
- **Bus Management**: Simple initialization for SPI2/SPI3 hosts.
- **Device Management**: Thread-safe device registration and handle tracking.
- **DMA Support**: Automatic DMA channel allocation for high-bandwidth peripherals.
- **Transfer Types**: Supports both blocking (polling/interrupt) and non-blocking (queued DMA) transfers.

## API
- `spi_bsp_bus_init()`: Setup the SPI host and GPIOs.
- `spi_bsp_add_device()`: Add a specific peripheral (e.g. ST7789) to the bus.
- `spi_bsp_transfer()`: Standard blocking write.
- `spi_bsp_queue_transfer()`: Queue a DMA transfer for background processing.
