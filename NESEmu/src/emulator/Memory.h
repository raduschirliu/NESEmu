#pragma once

#include "../debug/Logger.h"
#include <cstdint>

// Represents the 64KB of RAM that is addressable by the 6502 CPU
class Memory
{
public:
	// Initialize all empty memory blocks
	Memory();

	// Deallocate all memory
	~Memory();

	// Return pointer to a place in memory, or nullptr if out of bounds
	uint8_t *get(uint16_t address);
	
	// Return value of a place in memory, or -1 if not found
	uint8_t read(uint16_t address);

	// Set value in memory at address, or does nothing if out of bounds
	void set(uint16_t address, uint8_t value);

	// Reads and returns a NEW array containing range of memory, if it exists.
	// If not, will contain zeros. Array index 0 -> range start.
	uint8_t *readRange(uint16_t start, uint16_t end);

	// Dump entire contents of memory ($0000 - $FFFF) to given logger
	void dump(Logger &logger);

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
};