#include "emulator/CPU.h"
#include "emulator/PPU.h"
#include "util/Logger.h"
#include "emulator/NES.h"

#include <iostream>
#include <stdio.h>

int main(int argc, char **argv)
{
    NES nes;

    if (!nes.init())
    {
        return 1;
    }

    // nes.loadDebugMode();
    nes.load("roms/donkey-kong.nes");
    nes.run();

    return 0;
}