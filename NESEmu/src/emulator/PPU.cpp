#include "PPU.h"

PPU::PPU() : logger("..\\logs\\ppu.log")
{
	// TODO: Init pattern, name, and pallette tables

	// OAM is 256 bytes
	oam = new uint8_t[256];
}

PPU::~PPU()
{
	delete patternTables;
	delete nameTables;
	delete paletteTables;
	delete oam;
}

uint8_t *PPU::getMemory(uint16_t address)
{
	return nullptr;
}

uint8_t PPU::readMemory(uint16_t address)
{
	uint8_t *mem = getMemory(address);

	if (mem)
	{
		return *mem;
	}

	return 0;
}

void PPU::setMemory(uint16_t address, uint8_t value)
{
	uint8_t *mem = getMemory(address);

	if (mem)
	{
		*mem = value;
	}
}