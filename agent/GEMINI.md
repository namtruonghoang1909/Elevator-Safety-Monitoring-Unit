# ESMU Agent Context File

# This file specifies context and rules for an integrated AI agent to assist in the development of the Elevator Safety Monitoring Unit (ESMU).

### ------------------ PROJECT ABOUTS ------------------ ###
## Project Overview
- **Project Name**: Elevator Safety Monitoring Unit (ESMU)
- **Platform**: ESP32 (ESP-IDF Framework)
- **Build System**: PlatformIO
- **Target Board**: ESP32 DoIt DevKit V1

## Project Purpose
The ESMU monitors elevator safety
- detects:
    - elevator shakes that exceeds the limit
    - Emergency stops
    - abnormal speed changes

- send emergency calls, publish to a emergency mqtt topic when abnormal behavior detected.

- monitors:
    - elevator's health publish through mqtt

The system is safety-critical and must prioritize fail-safe behavior.

### Project Components
- Hardwares:
    - **MPU6050**: 6-axis accelerometer/gyroscope for motion sensing
    - **SSD1306**: 128x64 OLED display for visual output
    - **SIM7600**: sim module for sending emergency calls and sms(will be added later)
    - **I2C Bus**: Main communication protocol for sensors

- Firmware:
    - **display**: uses ssd1306 driver to display connection, elevator states.
    - **mqtt**: publish elevator's monitoring data.
    - **fault detector**: uses gyro/accel data to decides if abnormal behaviors occur
    - **web server**: local web for wifi credentials configuration.

## Project state
- system controller implementation has not done yet.
- mpu6050 and ssd1306 drivers have been implemented and tested.
- i2c platform (project-level) has been implemented and tested.

## Critical Rules
- before executing a task, create a todo list for that TASK in the agent/TODO.md, once the developer approved in the CLI, agent starts executing the task
- Explore the codebase and its contents before making any decisions.
- Before modifying the codes, files, explains detaily in the chat about the modification(why, where)
- After successfully fixing bugs, modification being accepted, list them in the MEMORY.md file with timestamps, what, where, why. Make it clear for later reviewing.

### ------------------ BUILD ACTIONS ------------------ ###
## Build Commands(bash)
# Build project
C:\Users\HOANGNAM\.platformio\penv\Scripts\platformio.exe run

# Flash firmware
C:\Users\HOANGNAM\.platformio\penv\Scripts\platformio.exe run --target upload --upload-port COM4

# Test Project
C:\Users\HOANGNAM\.platformio\penv\Scripts\platformio.exe test --upload-port COM4 --test-port COM4 

# Monitor serial output
C:\Users\HOANGNAM\.platformio\penv\Scripts\platformio.exe device monitor --port COM4

### ------------------ CODING RULES ------------------ ###

### File Organization
- Header files in `include/` subdirectory within each component
- Implementation files in component root

### Naming Conventions
- Functions: `component_action` (e.g., `ssd1306_init`, `mpu6050_read_accel`)
- Types: `snake_case_t` (e.g., `mpu6050_data_t`, `ssd1306_config_t`)
- Macros: `COMPONENT_ACTION` (uppercase with underscore)
- Enums: `COMPONENT_STATE` or `COMPONENT_FLAG`

### Documentation
- Use Doxygen-style comments for public APIs
- Include register-level documentation from hardware datasheets

## Important Files

### Entry Point
- `src/main.c`: Application entry point

### Key Configuration
- `platformio.ini`: PlatformIO build configuration
- `sdkconfig.esp32doit-devkit-v1`: ESP-IDF SDK configuration

## Git Workflow
- Branch naming: `feature/component-name` or `fix/issue-name`
- Use conventional commits: `feat()`, `fix()`, `refactor()`, `docs()`

### ------------------ DEBUGGING RULES ------------------ ###
- if any API we use returns a esp_err_t, we add a log code below that to check the return esp_err_t 
- any critial code sections should have ESP_LOGI to spectate the code behavior.

### ------------------ CHAT RULES ------------------ ###
- Instead of typing in the chat, user might type in the agent/task.md, then agent will be told to read the task.md

## Extra [commands]
- When user have a [command] infront of a prompt, agent has to do some desired works. This basically to let the agent knows what its supposed to do.
    - [chat] → Conversation only. No file modifications or code execution unless explicitly requested.
    - [discuss] → Deep conceptual discussion with optional web research.
    - [quick] → Short, direct answer only.
    - [design] → High-level architecture or system design. Stores the answer to agent/plan/idea.md, continue writing to the file, do not overwrite.
    - [code] -> Agent about to implement a new function, feature.
    - [refactor] → Improve code structure and modularity. Show the new code to the chat section, waits for the approval of the user.
    - [comment] → Add or fix comments based on the context that user provides after the command.
    - [debug] → Analyze logs/errors and find root cause.
    - [fix] → This command usually run after the [debug] command, once the errors have been found, agent attempts to create todolist on the agent/plan/FIX_BUG_PLAN.md,
    once the todolist was approved by the user, agent executes the todolist. continue writing to the file, do not overwrite.
    - [run] → This command usually followed by build, flash, test, serial monitor, agent will execute the commands based on the 
    provided commands in BUILD ACTIONS section then observe the output, report the thoughts about the output to the user.
    - [review] → Code or architecture critique.
    - [unit-test] → Generate proper unit tests (host-based if possible).
    - [integration-test] → Generate hardware-level or system tests.
    - [simulate_fails] → Simulate failure scenarios.
    - [doc] → Generate structured documentation.
    - [commit] → Generate commit / PR messages.
    - [resume] → Convert work into resume-ready bullet points.
    - [CHECKPOINT] → This usually called before the user's quit. Modify the agent/CHECKPOINT.md, stores things that the agent need to remember. 
    Next time the user opens the agent, agent will read agent/CHECKPOINT.md to understand the previous working context. 