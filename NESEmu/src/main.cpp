#include "emulator/CPU.h"
#include "emulator/PPU.h"
#include "debug/Logger.h"
#include "emulator/NES.h"

#include <iostream>
#include <stdio.h>
#include <chrono>

void testEmulator()
{
    Memory memory;
    PPU ppu(memory);
    CPU cpu(memory);

    ROM rom("..\\roms\\nestest.nes");
    rom.map(memory, cpu, ppu);

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

void timeEmulator()
{
    // Run headless NES
    NES nes;

    // nes.loadDebugMode();
    nes.load("..\\roms\\donkey-kong.nes");

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    using std::chrono::seconds;

    auto t1 = high_resolution_clock::now();

    // Run 2 mil steps (2mil CPU cycles, 6mil PPU cycles)
    uint64_t cpuCycles = 2000000;
    uint64_t ppuCycles = cpuCycles * 3;

    for (uint64_t i = 0; i < cpuCycles; i++)
    {
        nes.step();
    }

    auto t2 = high_resolution_clock::now();

    /* Getting number of seconds as a double. */
    duration<double> time = t2 - t1;

    std::cout << "Done\n";
    std::cout << "Time: " << time.count() << "s\n";

    // Should be 1.79Mhz
    double cpuMhz = cpuCycles / time.count() / 1000000;

    std::cout << "CPU speed: " << cpuMhz << "MHz \n";
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