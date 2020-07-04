#include <iostream>
#include <stdio.h>
#include "CPU.h"

int main()
{
    uint8_t bytes[] = { 0xe8, 0xe8, 0xe8, 0x8a, 0x38, 0x6a };

    Memory memory;
    
    for (int i = 0; i < sizeof(bytes) / sizeof(bytes[0]); i++)
    {
        memory.set(i, bytes[i]);
    }

    CPU cpu;
    cpu.setMemory(&memory);

    for (int i = 0; i < 14; i++)
    {
        cpu.step();
    }

    return 0;
}
