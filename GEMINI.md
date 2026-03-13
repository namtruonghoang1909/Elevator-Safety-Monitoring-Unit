# ESMU Agent Context File

# This file specifies context and rules for an integrated AI agent to assist in the development of the Elevator Safety Monitoring Unit (ESMU).

## Mandatory Initialization
- **At the start of every session, the agent MUST read**:
  1. `agent/context/roadmap.md` (To understand current high-level progress).
  2. `agent/context/checkpoint.md` (To resume the last task and context).
  3. All files in `agent/rules/` (To align with project-specific interaction, coding, and debugging standards).

### ------------------ PROJECT ABOUTS ------------------ ###
## Project Overview
- **Project Name**: Elevator Safety Monitoring Unit (ESMU)
- **Architecture**: Distributed Dual-Node System
- **Nodes**:
  - **Gateway Node**: ESP32 (ESP-IDF Framework) - Handles Connectivity, UI, and Communication.
  - **Edge Node**: STM32 (HAL Framework) - Handles Sensors and Fault Detection.
- **Communication Protocol**: CAN Bus (Standard CAN 2.0B)
- **Build Systems**: 
  - ESP32: PlatformIO
  - STM32: STM32CubeIDE / Makefile

## Project Purpose
The ESMU is a distributed safety-critical system for elevator monitoring:
- **Edge Node (STM32)**:
  - Continuously monitors MPU6050 (Accel/Gyro).
  - Performs real-time fault detection (Shakes, Emergency Stops, Free Fall).
  - Reports status to Gateway via CAN bus.
- **Gateway Node (ESP32)**:
  - Acts as a proxy for Edge data.
  - Manages SSD1306 Display for local status visualization.
  - Handles MQTT telemetry and WiFi/SIM7600 emergency alerts.

The system prioritizes three types of data over the CAN bus:
1. **Emergency (Highest Priority)**: Immediate interrupt signal when a fault is detected.
2. **Elevator Health (High Priority)**: 100ms task reporting motion and balance metrics.
3. **Edge Health (Medium Priority)**: 1s heartbeat task reporting node status.

### Project Components
- **Hardware**:
    - **MPU6050**: 6-axis accelerometer/gyroscope (on Edge Node).
    - **SSD1306**: 128x64 OLED display (on Gateway Node).
    - **SIM7600**: Cellular module for backup emergency alerts (on Gateway Node).
    - **CAN Transceivers**: For inter-node communication.

- **Firmware Services**:
    - **Gateway**: `display_service`, `connectivity_manager` (WiFi/MQTT), `can_manager`, `motion_proxy`.
    - **Edge**: `mpu6050_driver`, `fault_detector`, `can_manager`.

## Project State
- **Architecture Shift**: Migrating from single ESP32 node to Distributed ESP32+STM32.
- **Completed**: ESP32 I2C Platform, MPU6050 driver (ported to STM32 pending), SSD1306 driver, WiFi Manager, MQTT Manager.
- **In Progress**: `esmu_protocol.h` definition, CAN Bus implementation on both nodes, Motion Proxy refactor on ESP32.

## Critical Rules
- before executing a task, create a todo list for that TASK in the agent/temporary/task.md, once the developer approved in the CLI, agent starts executing the task
- Explore the entire agent/ directory and its contents before making any decisions.
- Before modifying the codes, files, explains detaily in the chat about the modification(why, where)
- After successfully fixing bugs, modification being accepted, list them in the agent/context/memory.md file with timestamps, what, where, why. Make it clear for later reviewing.


### ------------------ AGENT FOLDER STRUCTURE ------------------ ###
- components: Contains brief infomations about the components of the system.
- context: holds the working context for AI agent, current state of the project(roadmap.md), detailed context that developer was working on(checkpoint.md).
- memory: place to save encountered bugs and its solution(fixed_bugs.md), note for developer's correction for AI(self_improve.md).
- plan: place for AI agent to express the plans before building something, plan before fixing a bug(bug_fixing_plan), plan for designing system, modules(idea.md), plan before start implementing anything(code_plan.md).
- rules: this is the most crucial folder, agent must understand all contents in this folder before doing anything, it contains chat, code, debug, fix bug, document RULES.
- temporary: this folder is for temporary task from user if the task are too long(task.md)
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
