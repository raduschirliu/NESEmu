# nesemu

A simple Nintendo Entertainment System (NES) Emulator, written in C++.  
This project is not focused on being a perfectly accurate emulation of the NES and its hardware, or to be fully-featured. It is a project meant for learning, with the goal being for it to be "good enough" to play some common games, while being clean and easy to understand.

## Features

#### Planned features

- [ ] 6502 CPU
    - [x] Registers and memory map
    - [x] All addressing modes
    - [ ] All official opcodes
- [ ] Mappers
    - [ ] NROM (#000)
    - [ ] MMC1 (#001)
- [ ] PPU
    - [ ] Rendering context (likely using SDL)
    - [ ] Memory mapped registers
    - [ ] Rendering loop

#### Extra features

- [ ] CPU
    - [ ] Cycle accurate emulation
    - [ ] Unofficial opcodes
- [ ] APU emulation
- [ ] Fast forward, run, pause modes
- [ ] Save states
- [ ] Mappers
    - [ ] UxROM (#002)
    - [ ] Mapper 3 (#003)
    - [ ] MMC 3 (#004)
- [ ] Debugger
    - [ ] Memory viewer
    - [ ] Stack viewer
    - [ ] Stepping through CPU instructions
    - [ ] Palette viewer
    - [ ] Pattern table viewer