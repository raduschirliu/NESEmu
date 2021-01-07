#include <iostream>
#include <stdio.h>
#include "CPU.h"

int main()
{
    Memory memory;
    
    ROM rom("C:\\Dev\\Projects\\nesemu\\roms\\nestest.nes");
    rom.read(&memory);
    printf("Mapper: %u\n", rom.getMapperID());
    printf("ROM size: %u\n", rom.header.prgBanks * 0x4000);

    CPU cpu(&memory);
    cpu.setPC(0xC000);

    // 26554 cycles for entire NESTest ROM
    for (int i = 0; i < 26554; i++)
    {
        cpu.step();
    }

    printf("Executed 26554 cycles of NESTest ROM\n");

    printf("Test results:\n");
    printf("\t0x02: %02X\n", memory.read(0x02));
    printf("\t0x03: %02X\n", memory.read(0x03));

    return 0;
}