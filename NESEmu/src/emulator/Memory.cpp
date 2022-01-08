#include "Memory.h"
#include <stdio.h>
#include <iomanip>
#include <sstream>

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

uint8_t *Memory::get(uint16_t address, bool write, bool skipCallback)
{
	if (address >= 0x0000 && address <= 0x1FFF)
	{
		// Get from CPU memory ($0000 - $1FFF, mirrored > $07FF)
		uint16_t target = address % 0x0800;
		return &cpuMem[target];
	}
	else if (address >= 0x2000 && address <= 0x3FFF)
	{
		// Get from PPU memory ($2000 - $3FFF, mirrored > $2008)
		uint16_t target = (address - 0x2000) % 0x0009;

		if (ppuCallback && !skipCallback)
		{
			// TODO: Add way of determining intent to write to address
			// TODO: Add way to access memory without triggering callbacks
			ppuCallback(address, 0, write);
		}

		return &ppuMem[target];
	}
	else if (address >= 0x4000 && address <= 0x4017)
	{
		// Get from APU & I/O Memory ($4000 - $4017)
		return &apuMem[address - 0x4000];
	}
	else if (address >= 0x4018 && address <= 0x401F)
	{
		// Get from APU & I/O test Memory (usually disabled) ($4018 - $401F)
		return &testMem[address - 0x4018];
	}
	else if (address >= 0x4020 && address <= 0xFFFF)
	{
		// Get from cartridge Memory ($4020 - $FFFF)
		return &romMem[address - 0x4020];
	}

	return nullptr;
}

uint8_t Memory::read(uint16_t address, bool skipCallback)
{
	uint8_t *ptr = get(address, false, skipCallback);

	if (ptr != nullptr)
	{
		return *ptr;
	}

	return -1;
}

void Memory::write(uint16_t address, uint8_t value, bool skipCallback)
{
	uint8_t *ptr = get(address, true, skipCallback);

	if (ptr != nullptr)
	{
		*ptr = value;
	}
}

void Memory::dump(Logger &logger)
{
	std::stringstream ss;

	ss << "Full memory dump\n"
		<< "--------------------\n"
		<< std::hex;

	// Dump 8 bytes per line
	for (int base = 0; base <= 0xFFFF; base += 8)
	{
		ss << std::setw(4) << std::setfill('0')
			<< base << ":\t";

		for (int line = 0; line < 8; line++)
		{
			ss << std::setw(2) << std::setfill('0')
				<< (int)read(base + line) << " ";
		}

		ss << std::endl;
	}

	logger.write(ss.str());
}

void Memory::setPpuAccessCallback(AccessCallback callback)
{
	ppuCallback = callback;
}