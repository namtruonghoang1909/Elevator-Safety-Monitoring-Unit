# Task: Move system_registry to app/ and Separate app/core into Folders

## Context
As requested, we will move `system_registry` into `app/` and organize `app/core/` into functional subfolders. Headers used outside their folder will be in `include/`, while internal headers will remain in the subfolder root.

## Proposed Structure
```text
app/
├── registry/
│   ├── include/
│   │   └── system_registry.h
│   ├── system_registry.c
│   └── CMakeLists.txt
└── core/
    ├── boot/
    │   ├── system_boot.c
    │   └── system_boot.h (Internal)
    ├── controller/
    │   ├── include/
    │   │   └── system.h (Used by main.c)
    │   ├── system.c
    │   ├── system_controller.c
    │   └── system_event.h (Internal)
    ├── hw/
    │   ├── system_hw.c
    │   └── system_hw.h (Internal)
    ├── heartbeat/
    │   ├── heartbeat.c
    │   └── heartbeat.h (Internal)
    ├── include/
    │   └── system_config.h (Core-wide config)
    └── CMakeLists.txt
```

## Todo List
### Part 1: Move System Registry
- [ ] Move `middleware/registry/` to `app/registry/`
- [ ] Move `app/registry/system_registry.h` to `app/registry/include/`
- [ ] Update `app/registry/CMakeLists.txt`
- [ ] Update `gateway-esp32/CMakeLists.txt` (if it was explicitly listed there, otherwise it's just a component discovery)

### Part 2: Refactor app/core
- [ ] Create `app/core/boot/` and move `system_boot.*`
- [ ] Create `app/core/controller/include/` and move `system.h`
- [ ] Create `app/core/controller/` and move `system.c`, `system_controller.c`, `system_event.h`
- [ ] Create `app/core/hw/` and move `system_hw.*`
- [ ] Create `app/core/heartbeat/` and move `heartbeat.*`
- [ ] Create `app/core/include/` and move `system_config.h`
- [ ] Update `app/core/CMakeLists.txt` to reflect the new paths and include dirs
- [ ] Update internal includes in all core files

### Part 3: Verification
- [ ] Verify build stability with `pio run`
