#include <iostream>
#include <stdio.h>
#include "CPU.h"

int main()
{
    Memory memory;
    /*
    memory.set(0, 0xAD);
    memory.set(1, 0xA0);
    memory.set(2, 0x16);

    memory.set(1000, 0x52);
    memory.set(1001, 0x3A);

    memory.set(0, 0x4C);
    memory.set(1, 0x00);
    memory.set(2, 0x10);
    */

    // memory.set(0, 0xCA);

    memory.set(0, 0x30);
    memory.set(1, 0x07);
    
    for (int i = 0; i < 8; i++)
    {
        memory.set(2 + i, 0xCA);
    }

    CPU cpu;
    cpu.setMemory(&memory);
    //cpu.step();
    //cpu.step();
    //cpu.step();

    //cpu.step();
    //cpu.step();
    //cpu.step();
    //cpu.step();

    return 0;
}
