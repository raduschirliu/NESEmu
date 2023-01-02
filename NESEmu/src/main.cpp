#include <stdio.h>

#include <iostream>

#include "emulator/CPU.h"
#include "emulator/NES.h"
#include "emulator/PPU.h"
#include "util/Logger.h"

int main(int argc, char **argv)
{
    NES nes;

    if (!nes.init())
    {
        return 1;
    }

    // nes.loadDebugMode();

    if (argc != 2)
    {
        printf("Usage: nesemu <ROM path>");
        return 1;
    }

    nes.load(argv[1]);
    nes.run();

    return 0;
}