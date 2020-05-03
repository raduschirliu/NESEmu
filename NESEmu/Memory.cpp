#include "Memory.h"

Memory::Memory()
{

}

void Memory::cpuWrite(uint8_t address, uint8_t data)
{
	if (address >= 0 && address <= 0x1FFF)
	{
		// Wrap around when accessing memory > $07FF
		address %= 0x0800;
		cpuRam[address] = data;
	}
	else
	{
		// Address not in CPU RAM range
	}
}

uint8_t Memory::cpuRead(uint8_t address) const
{
	if (address >= 0 && address <= 0x1FFF)
	{
		// Wrap around when accessing memory > $07FF
		address %= 0x0800;
		return cpuRam[address];
	}
	else
	{
		// Address not in CPU RAM range
		return -1;
	}
}