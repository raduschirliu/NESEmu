#pragma once

#include <cstdint>

class Memory
{
public:
	Memory();
	~Memory();

	// Return pointer to a place in memory, or nullptr if out of bounds
	uint8_t *get(uint16_t address);
	
	// Return value of a place in memory, or -1 if not found
	uint8_t read(uint16_t address);

	// Set value in memory at address, or does nothing if out of bounds
	void set(uint16_t address, uint8_t value);

private:
	// CPU Memory from $0000 - $07FFF
	// Mirrored 3 times from $0800 - $1FFF
	uint8_t *cpuMem;

	// PPU Memory from $2000 - $2007
	// Mirrored every 8 bytes from $2008 - $3FFF
	uint8_t *ppuMem;

	// APU & I/O Memory from $4000 - $4017
	uint8_t *apuMem;

	// APU & I/O Memory (usually disabled) from $4018 - $401F
	uint8_t *testMem;

	// Cartridge Memory, PRG ROM, PRG RAM, Mappers, from $4020 - $FFFF
	uint8_t *romMem;
};