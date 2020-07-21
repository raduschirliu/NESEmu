#include <iostream>
#include <stdio.h>
#include "CPU.h"

int main()
{
    Memory memory;
    
    ROM rom("C:\\Dev\\Projects\\nesemu\\roms\\nestest.nes");
    rom.read(&memory);

    CPU cpu;
    cpu.setMemory(&memory);
    cpu.setPC(0xC000);

    // 26554 cycles for all
    for (int i = 0; i < 180; i++)
    {
        cpu.step();
    }

    return 0;
}