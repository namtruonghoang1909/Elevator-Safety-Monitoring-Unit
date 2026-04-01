### ------------------ CODING RULES ------------------ ###

### File Organization
- Header files in `include/` subdirectory within each component
- Implementation files in component root
- Codes should be separated into files and folders for better understanding and reviewing.
- Source file's variables, functions should be organized like this:
// ─────────────────────────────────────────────
// Private variables, macros
// ─────────────────────────────────────────────
....
// ─────────────────────────────────────────────
// Private helpers declarations or definitions
// ─────────────────────────────────────────────
static function declarations or definition ....

// ─────────────────────────────────────────────
// Global function declarations or definitions
// ─────────────────────────────────────────────
function declarations or definitions ....

### Naming Conventions
- For global APIs: `component_action` (e.g., `ssd1306_init`, `mpu6050_read_accel`)
- For static functions: `action` (e.g., `apply_filter`, `state_transition`)
- Types: `snake_case_t` (e.g., `mpu6050_data_t`, `ssd1306_config_t`)
- Macros: `COMPONENT_ACTION` (uppercase with underscore)
- Enums: `COMPONENT_STATE` or `COMPONENT_FLAG`

### Comments
- A header or source file should have a brief description about the code inside it.
- Every function declaration should have a detail comments about its return value, arguments, what it does, what should be called before or after it.
- Before every code segment that does a specific work, add a short comment that describes the flow. 


### Important Files

### Entry Point
- `src/main.c`: Application entry point

### Key Configuration
- `platformio.ini`: PlatformIO build configuration
- `sdkconfig.esp32doit-devkit-v1`: ESP-IDF SDK configuration

