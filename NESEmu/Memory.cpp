#include "Memory.h"
#include <stdio.h>

Memory::Memory()
{
	// Allocate memory arrays for all NES components
	cpuMem = new uint8_t[2048];
	ppuMem = new uint8_t[8];
	apuMem = new uint8_t[24];
	testMem = new uint8_t[8];
	romMem = new uint8_t[49120];
}

Memory::~Memory()
{
	// Deallocate all memory arrays
	delete[] cpuMem;
	delete[] ppuMem;
	delete[] apuMem;
	delete[] testMem;
	delete[] romMem;
}

uint8_t *Memory::get(uint16_t address)
{
	if (address >= 0x0000 && address <= 0x1FFF)
	{
		// Get from CPU memory ($0000 - $1FFF, mirrored > $07FF)
		address %= 0x0800;
		return &cpuMem[address];
	}
	else if (address >= 0x2000 && address <= 0x3FFF)
	{
		// Get from PPU memory ($2000 - $3FFF, mirrored > $2008)
	}
	else if (address >= 0x4020 && address <= 0xFFFF)
	{
		// Get from cartridge Memory ($4020 - $FFFF)
		return &romMem[address - 0x4020];
	}

	// TODO: Implement the rest
	return nullptr;
}

uint8_t Memory::read(uint16_t address)
{
	uint8_t *ptr = get(address);

	if (ptr != nullptr)
	{
		return *ptr;
	}

	return -1;
}

void Memory::set(uint16_t address, uint8_t value)
{
	uint8_t *ptr = get(address);

	if (ptr != nullptr)
	{
		*ptr = value;
	}
}
