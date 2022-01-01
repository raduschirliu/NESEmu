#include "PPU.h"

#include <fstream>

PPU::PPU(Memory &memory) : logger("..\\logs\\ppu.log"), memory(memory)
{
	// TODO: Properly patternTable, nameTable
	patternTables = new uint8_t[0x2000]; // TEST: Init pattern table to 16KB of memory
	nameTables = new uint8_t[0x1000];	 // TEST: Init just to be able to dump memory (8KB of memory)

	paletteTables = new uint8_t[0x20]; // 32 bytes, not configurable/remapable
	oam = new uint8_t[256];			   // 256 bytes, internal PPU memory

	// Make PPU registers point to CPU memory
	registers = reinterpret_cast<Registers *>(memory.get(0x2000));

	// Load system palette from .pal file
	loadPalette("palette.pal");
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

std::vector<PPU::Color> PPU::getSystemPalette()
{
	return systemPalette;
}

void PPU::loadPalette(std::string path)
{
	std::ifstream stream;
	stream.open(path, std::ifstream::binary);

	if (!stream.is_open())
	{
		printf("Error, failed to load palette: %s\n", path.c_str());
		return;
	}

	Color color;

	// Read first 64 palette color entries
	while (!stream.eof() && systemPalette.size() < 64)
	{
		stream.read((char *)&color, 3);
		systemPalette.push_back(color);
	}

	printf("Loaded %u system palette colors\n", systemPalette.size());
}