#pragma once

#include <cstdint>

class Memory
{
public:
	Memory();

	// Return pointer to a place in memory
	uint8_t *get(uint16_t address);
	
	// Return value of a place in memory
	uint8_t read(uint16_t address);

	// Set value in memory at address
	void set(uint16_t address, uint8_t value);

private:
	// CPU Memory from $0000 - $07FFF
	// Mirrored 3 times from $0800 - $1FFF
	uint8_t cpuMem[2048];

	// PPU Memory from $2000 - $2007
	// Mirrored every 8 bytes from $2008 - $3FFF
	uint8_t ppuMem[8];

	// APU & I/O Memory from $4000 - $4017
	uint8_t apuMem[24];

	// APU & I/O Memory (usually disabled) from $4018 - $401F
	uint8_t testMem[8];

	// Cartridge Memory, PRG ROM, PRG RAM, Mappers, from $4020 - $FFFF
	uint8_t romMem[49120];
};