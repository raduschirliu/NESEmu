#include "emulator/CPU.h"
#include "emulator/PPU.h"
#include "util/Logger.h"
#include "emulator/NES.h"

#include <iostream>
#include <stdio.h>

void testEmulator()
{
    Bus bus;
    PPU ppu(bus);
    CPU cpu(bus);

    Cartridge cartridge;
    cartridge.load("..\\roms\\nestest.nes");

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
    printf("\t0x02: %02X\n", bus.read(0x02));
    printf("\t0x03: %02X\n", bus.read(0x03));

    // Dump memory post-run
    Logger memlog("..\\logs\\post-run-memory.log");
    bus.dump(memlog);
    printf("Memory dumped post-run");
}

int runEmulator()
{
    NES nes;

    if (!nes.init())
    {
        return 1;
    }

    // nes.loadDebugMode();
    nes.load("..\\roms\\donkey-kong.nes");
    nes.run();

    return 0;
}

int main(int argc, char **argv)
{
    return runEmulator();
}