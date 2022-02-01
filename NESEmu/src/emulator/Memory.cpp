#include "Memory.h"
#include <stdio.h>
#include <iomanip>
#include <sstream>

// TODO: Class should be renamed to "Bus" and be connected to PPU

// PPU register locations
static uint16_t constexpr PPU_OAMDATA = 0x4014;

Bus::Bus()
{
	// Allocate memory arrays for all NES components
	cpuMem = new uint8_t[2048]();
	ppuMem = new uint8_t[8]();
	apuMem = new uint8_t[24]();
	testMem = new uint8_t[8]();
	romMem = new uint8_t[49120]();
	
	// Other things needed on the bus
	shouldDispatchNmi = false;
	shouldDispatchOamTransfer = false;
}

Bus::~Bus()
{
	// Deallocate all memory arrays
	delete[] cpuMem;
	delete[] ppuMem;
	delete[] apuMem;
	delete[] testMem;
	delete[] romMem;
}

uint8_t *Bus::get(uint16_t address)
{
	if (address >= 0x0000 && address <= 0x1FFF)
	{
		// Get from CPU memory ($0000 - $1FFF, mirrored > $07FF)
		uint16_t target = address % 0x0800;
		return &cpuMem[target];
	}
	else if (address >= 0x2000 && address <= 0x3FFF)
	{
		// Get from PPU memory ($2000 - $3FFF, mirrored > $2007)
		uint16_t target = (address - 0x2000) % 0x0008;
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

uint8_t Bus::read(uint16_t address, bool skipCallback)
{
	uint8_t *ptr = get(address);

	if (ptr != nullptr)
	{
		uint8_t val = *ptr;
		
		if (!skipCallback)
		{
			dispatchCallbacks(address, 0, false);
		}

		return val;
	}

	// TODO: Return an error value?
	return 0;
}

void Bus::write(uint16_t address, uint8_t value, bool skipCallback)
{
	uint8_t *ptr = get(address);

	if (ptr != nullptr)
	{
		*ptr = value;

		if (!skipCallback)
		{
			dispatchCallbacks(address, value, true);
		}
	}
}

void Bus::dump(Logger &logger)
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

void Bus::setPpuAccessCallback(AccessCallback callback)
{
	ppuMemoryAccessCallback = callback;
}

void Bus::setPpuOamTransferCallback(OamTransferCallback callback)
{
	ppuOamTransferCallback = callback;
}

void Bus::dispatchNmi()
{
	shouldDispatchNmi = true;
}

void Bus::dispatchOamTransfer()
{
	if (ppuOamTransferCallback)
	{
		// Will read from CPU memory $XX00 - $XXFF
		uint16_t address = read(PPU_OAMDATA) << 8;
		uint8_t *data = get(address);
		ppuOamTransferCallback(data);
	}

	shouldDispatchOamTransfer = false;
}

bool Bus::pollNmi()
{
	if (shouldDispatchNmi)
	{
		shouldDispatchNmi = false;
		return true;
	}

	return false;
}

bool Bus::pollOamTransfer()
{
	return shouldDispatchOamTransfer;
}

void Bus::dispatchCallbacks(uint16_t address, uint8_t value, bool write)
{
	if (address >= 0x2000 && address <= 0x3FFF)
	{
		// Accessing PPU memory ($2000 - $3FFF, mirrored > $2007)
		// Normalize target to non-mirrored register range ($2000 - $2007)
		uint16_t target = (address - 0x2000) % 0x0008 + 0x2000;

		if (ppuMemoryAccessCallback)
		{
			ppuMemoryAccessCallback(target, value, write);
		}
	}
	else if (address == 0x4014)
	{
		// Accessing OAMDMA for PPU
		if (ppuMemoryAccessCallback)
		{
			ppuMemoryAccessCallback(0x4014, value, write);
			
			if (write)
			{
				shouldDispatchOamTransfer = true;
			}
		}
	}
}