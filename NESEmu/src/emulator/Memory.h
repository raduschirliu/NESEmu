#pragma once

#include "../debug/Logger.h"

#include <cstdint>
#include <functional>

// Represents the 64KB of RAM that is addressable by the 6502 CPU
class Memory
{
public:
	// Callback type
	// using AccessCallback = void(*)(uint16_t address, uint8_t newValue, bool write);
	using AccessCallback = std::function<void(uint16_t address, uint8_t newValue, bool write)>;

	// Initialize all empty memory blocks
	Memory();

	// Deallocate all memory
	~Memory();

	// Return pointer to a place in memory, or nullptr if out of bounds
	uint8_t *get(uint16_t address, bool write, bool skipCallback = false);
	
	// Return value of a place in memory, or -1 if not found
	uint8_t read(uint16_t address, bool skipCallback = false);

	// Write value in memory at address, or does nothing if out of bounds
	void write(uint16_t address, uint8_t value, bool skipCallback = false);

	// Dump entire contents of memory ($0000 - $FFFF) to given logger
	void dump(Logger &logger);

	// Set the PPU memory access callback
	void setPpuAccessCallback(AccessCallback callback);

private:
	// Internal 2KB of CPU Memory (from $0000 - $07FFF)
	// Mirrored 3 times from $0800 - $1FFF
	uint8_t *cpuMem;

	// PPU Memory mapped registers (from $2000 - $2007)
	// Mirrored every 8 bytes from $2008 - $3FFF
	uint8_t *ppuMem;

	// APU & I/O Memory (from $4000 - $4017)
	uint8_t *apuMem;

	// APU & I/O Memory - usually disabled (from $4018 - $401F)
	uint8_t *testMem;

	// Cartridge Memory: PRG ROM, PRG RAM, and mapper registers (from $4020 - $FFFF)
	uint8_t *romMem;

	// Callbacks for when regions of memory are accessed
	AccessCallback ppuCallback;
};