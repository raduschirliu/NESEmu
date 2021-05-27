#include "emulator/CPU.h"
#include "emulator/PPU.h"
#include "debug/Logger.h"
#include "emulator/NES.h"

#include <iostream>
#include <stdio.h>

void testEmulator()
{
    PPU ppu;
    Memory memory;

    ROM rom("..\\roms\\nestest.nes");
    rom.map(&memory, &ppu);

    CPU cpu(&memory);

    printf("Mapper: %u\n", rom.getMapperID());
    printf("ROM size: %u\n", rom.header.prgBanks * 0x4000);

    // Set PC to 0xC000 to put NESTest rom into "automation" mode
    cpu.setPC(0xC000);

    // 26554 cycles for entire NESTest ROM
    for (int i = 0; i < 26554; i++)
    {
        cpu.step();
    }

    printf("Executed 26554 cycles of NESTest ROM\n");

    // Read NESTest test result codes
    printf("Test results:\n");
    printf("\t0x02: %02X\n", memory.read(0x02));
    printf("\t0x03: %02X\n", memory.read(0x03));

    // Dump memory post-run
    Logger memlog("..\\logs\\post-run-memory.log");
    memory.dump(memlog);
    printf("Memory dumped post-run");
}

int main(int argc, char **argv)
{
    NES nes;

    if (!nes.init())
    {
        return 1;
    }

    nes.load("..\\roms\\nestest.nes");
    nes.run();

    return 0;
}