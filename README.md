# nesemu

A simple Nintendo Entertainment System (NES) emulator, written in C++ and built using OpenGL, [ImGui](https://github.com/ocornut/imgui), GLEW, and GLFW.

This project is not focused on being a perfectly accurate emulation of the NES and its hardware, or to be fully-featured. It is a project meant for learning, with the goal being for it to be "good enough" to play some common games, while having code that is clean and easy to understand.

_Note: Nesemu is still very much WIP_

## Features

#### Planned features

- [x] CPU
    - [x] Registers and memory map
    - [x] All addressing modes
    - [x] All official opcodes
- [ ] Mappers
    - [ ] NROM (#000)
    - [ ] MMC1 (#001)
- [ ] PPU
    - [ ] Rendering context (likely using SDL)
    - [ ] Memory mapped registers
    - [ ] Rendering loop
- [ ] Debugger
    - [x] Memory viewer
    - [x] Stepping through CPU instructions
    - [ ] Stack viewer
    - [ ] Palette viewer
    - [ ] Pattern table viewer

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

## Building

The project is being developed in C++ using Visual Studio 2019, with a few scripts written using Python 3.
The only dependencies of the project (ImGui, GLEW, and GLFW) are included in the `libraries` directory.  

So far, it has only been tested under Windows 10, and can be built from within VS after the project has been imported. All dependencies are contained within the repo, and are referenced locally.