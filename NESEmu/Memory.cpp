#include "Memory.h"

Memory::Memory()
{
	// TODO: Move arrays to heap
}

uint8_t *Memory::get(uint16_t address)
{
	if (address >= 0x0000 && address <= 0x1FFF)
	{
		// Get from CPU memory ($0000 - $1FFF, mirrored > $07FF)
		address %= 0x0800;
		return &cpuMem[address];
	}
	else if (address >= 0x2000 && address <= 0x200)
	{
		// Get from PPU memory ($2000 - $3FFF, mirrored > $2008)
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
