# Display Service (TFT Dashboard)

The **Display Service** manages the ST7789 2.0" IPS TFT display on the ESMU Gateway. It provides real-time visualization of the system's state, connectivity metrics, and elevator motion data.

## 🚀 Key Features

- **State-Aware UI**: Automatically switches between the **Dashboard** (Monitoring) and the **Config Screen** (Provisioning).
- **Interactive Metrics**: Displays real-time vibration (g), speed (mm/s), motion state, and health.
- **Connectivity Dashboard**: Shows WiFi signal bars, MQTT status, and Cellular (4G) network info.
- **Emergency Overlays**: Provides a high-contrast inverse video overlay when a critical fault is active.
- **Boot Feedback**: Visual progress indicators during the system startup sequence.

## 🏗️ Modular Design

| Module | Location | Responsibility |
| :--- | :--- | :--- |
| **Public API** | `display.c` | External interface and task initialization. |
| **Core Task** | `display.c` | Periodically refreshes the screen (5-10Hz) based on registry snapshots. |
| **Dashboard** | `ui/dashboard.c` | Implementation of the main monitoring interface. |
| **Config Screen**| `ui/config_screen.c`| Implementation of the Web Config mode UI. |
| **Widgets** | `ui/widgets/` | Reusable UI components (Signal bars, Progress bars, Icons). |

## 🔧 Public API

- **`display_start()`**: Launches the background display task.
- **`display_set_brightness(pct)`**: Adjusts the PWM backlight (0-100%).
- **`dashboard_show()`**: Explicitly switches the UI to the metrics dashboard.
- **`config_screen_show()`**: Explicitly switches the UI to the provisioning instructions.

## 📂 Dependencies
- `st7789`: Low-level SPI TFT driver.
- `system_registry`: Primary data source for all UI elements.
- `pwm_platform`: For backlight dimming control.
