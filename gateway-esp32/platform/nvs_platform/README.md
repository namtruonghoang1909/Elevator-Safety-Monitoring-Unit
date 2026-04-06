# NVS Platform (Storage)

The **NVS Platform** is a low-level abstraction for ESP32 Non-Volatile Storage (NVS). It provides persistent storage for system-wide configuration that must survive power cycles.

## 🚀 Key Features

- **WiFi Persistence**: Stores and retrieves SSID and Password.
- **Emergency Configuration**: Stores and retrieves the user-defined emergency phone number.
- **Thread-Safety**: Uses the underlying ESP-IDF NVS locking mechanisms for safe concurrent access.
- **Robustness**: Automatically erases and re-initializes NVS if data corruption is detected.

## 🔧 Public API

- **`platform_nvs_init()`**: Initializes the NVS flash partition.
- **`platform_nvs_save_wifi_creds(ssid, pass)`**: Saves WiFi credentials to the `esmu_storage` namespace.
- **`platform_nvs_load_wifi_creds(ssid, ssid_len, pass, pass_len)`**: Retrieves stored WiFi credentials.
- **`platform_nvs_save_emergency_phone(phone)`**: Saves the target phone number for cellular alerts.
- **`platform_nvs_load_emergency_phone(phone, len)`**: Retrieves the stored emergency phone number.
- **`platform_nvs_clear_wifi_creds()`**: Erases all configuration data from NVS.

## 📂 Dependencies
- `nvs_flash`: ESP-IDF NVS flash component.
- `nvs`: ESP-IDF NVS key-value storage.
