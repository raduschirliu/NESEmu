#include <iostream>
#include <stdio.h>
#include "CPU.h"

int main()
{
    Memory memory;
    
    ROM rom("C:\\Dev\\Projects\\nesemu\\roms\\nestest.nes");
    rom.read(&memory);

    CPU cpu(&memory);
    cpu.setPC(0xC000);

    // 26554 cycles for entire NESTest ROM
    for (int i = 0; i < 26554; i++)
    {
        cpu.step();
    }

    printf("Executed 26554 cycles of NESTest ROM\n");

    return 0;
}