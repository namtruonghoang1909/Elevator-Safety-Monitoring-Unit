# I2C Platform Abstraction

This component provides a thread-safe, high-level abstraction over the ESP-IDF `i2c_master.h` driver (v5.x). It manages I2C buses and devices, ensuring multiple components can share the same physical bus without collision.

## Features

- **Multi-Bus Support**: Manage multiple I2C master buses concurrently.
- **Dynamic Device Registration**: Register and address devices by a simple `uint8_t` ID.
- **Thread Safety**: Internal mutexes (via ESP-IDF driver) ensure safe access from multiple tasks.
- **Error Diagnostics**: Comprehensive logging of I2C errors (NACK, Timeout, etc.) with register-level details.
- **Scan Utility**: Built-in function to scan for connected devices.

## Public API

### Bus Management
- `i2c_bus_init(bus_id_out, sda, scl)`: Initialize a physical bus.
- `i2c_scan(bus_id, addr_list, max, count)`: Probe the bus for devices.

### Device Management
- `i2c_add_device(dev_id_out, addr, name, speed, bus_id)`: Register a slave device.
- `i2c_remove_device(dev_id)`: Unregister and free resources.

### Data Transfer
- `i2c_write_reg(dev_id, reg, val)`: Write a single byte to a register.
- `i2c_read_reg(dev_id, reg, val_out)`: Read a single byte from a register.
- `i2c_read_consecutive_regs(dev_id, start_reg, buf, len)`: Burst read multiple registers.
- `i2c_write_bytes(dev_id, data, len)`: Direct stream write.

## Usage Example

```c
uint8_t bus_id;
i2c_bus_init(&bus_id, GPIO_NUM_21, GPIO_NUM_22);

uint8_t mpu_id;
i2c_add_device(&mpu_id, 0x68, "MPU6050", 400000, bus_id);

uint8_t who_am_i;
i2c_read_reg(mpu_id, 0x75, &who_am_i);
```
