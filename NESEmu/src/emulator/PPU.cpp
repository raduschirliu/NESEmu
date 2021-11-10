#include "PPU.h"

PPU::PPU(Memory &memory) : logger("..\\logs\\ppu.log"), memory(memory)
{
	// TODO: Properly patternTable, nameTable
	patternTables = nullptr;
	nameTables = nullptr;

	paletteTables = new uint8_t[0x20]; // 32 bytes, not configurable/remapable
	oam = new uint8_t[256]; // 256 bytes, internal PPU memory

	// Set PPU registers from CPU memory
	registers = reinterpret_cast<Registers *>(memory.get(0x2000));
}

PPU::~PPU()
{
	// TODO: Cleanup nameTable and patternTable if necessary
	delete[] paletteTables;
	delete[] oam;
}

void PPU::step()
{
	// TODO: Implement render cycles
}

uint8_t *PPU::getMemory(uint16_t address)
{
	if (address >= 0 && address <= 0x1FFF)
	{
		// Pattern table from 0x0000 - 0x1FFF
		return &patternTables[address];
	}
	else if (address >= 0x2000 && address <= 0x2FFF)
	{
		// Name tables from 0x2000 - 0x2FFF
		uint16_t offset = address - 0x1000;
		return &nameTables[offset];
	}
	else if (address >= 0x3000 && address <= 0x3EFF)
	{
		// Mirrors memory region 0x2000 - 0x2EFF
		uint16_t offset = address - 0x3000;
		return &nameTables[offset];
	}
	else if (address >= 0x3F00 && address <= 0x3FFF)
	{
		// Region 0x3F00 - 0x3F1F maps to palette RAM
		// Region 0x3F20 - 0x3FFF mirrors region above
		uint16_t offset = (address - 0x3F00) % 0x20;
		return &paletteTables[offset];
	}

	// Invalid address
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

PPU::Registers *PPU::getRegisters()
{
	return registers;
}