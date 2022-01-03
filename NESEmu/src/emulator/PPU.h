#pragma once

#include "../debug/Logger.h"
#include "Memory.h"

#include <vector>

class PPU
{
public:
	// Represents a system palette color
	struct Color
	{
		uint8_t r, g, b;
	};

	// Represents a 4 byte sprite stored in the Object Attribute Memory (OAM)
	struct OamSprite
	{
		uint8_t yPos;
		uint8_t tileIndex;
		uint8_t attributes;
		uint8_t xPos;
	};

	// Represents the PPU registers (0x2000 - 0x2007)
	struct Registers
	{
		struct PpuCtrl {
			uint8_t nmiEnable : 1;
			uint8_t masterSlave : 1;
			uint8_t spriteSize : 1;
			uint8_t bgPatternTable : 1;
			uint8_t spritePatternTable : 1;
			uint8_t addressIncrement : 1;
			uint8_t baseNametable : 2;
		} ctrl;
		uint8_t mask;
		uint8_t status;
		uint8_t oamAddr;
		uint8_t oamData;
		uint8_t scroll;
		uint8_t addr;
		uint8_t data;
	};

	// Initialize memory
	PPU(Memory &memory);
	
	// Cleanup memory
	~PPU();

	// Emulate one PPU cycle
	void step();

	// Returns pointer to memory location
	uint8_t *getMemory(uint16_t address);

	// Reads from memory location
	uint8_t readMemory(uint16_t address);

	// Sets value in memory location
	void setMemory(uint16_t address, uint8_t value);

	// Gets current value of PPU registers
	Registers *getRegisters();

	// Returns the system palette
	std::vector<PPU::Color> getSystemPalette();

	// Return the total amount of cycles that have passed
	uint32_t getTotalCycles();

private:
	// Cycle related stats
	uint8_t cycles;
	uint32_t totalCycles;

	// Used for pattern tables, $0000 - $1FFF. Each pattern table being $1000 in size
	// Mapped to cartridge using bank switching
	uint8_t *patternTables;

	// Used for name tables, $2000 - $2FFF. Normally mapped to 2kB of internal NES VRAM
	uint8_t *nameTables;

	// Used to hold palette control information, $3F00 - $3FFF. Region $3F20 - $3FFF mirrors $3F00 - $3F1F
	uint8_t *paletteTables;
	
	// Internal Object Attribute Memory (256kB)
	uint8_t *oam;

	// PPU registers from CPU memory
	Registers *registers;

	// System color palette
	std::vector<Color> systemPalette;
	
	// Log PPU activities
	Logger logger;

	// CPU memory
	Memory &memory;

	// Control the address that the CPU can access through PPUADDR/PPUDATA
	uint16_t accessAddress;
	bool accessAddressHighByte;

	// Load the system palette
	void loadPalette(std::string path);

	// Called when one of the PPU's memory mapped registers is accessed
	void onRegisterAccess(uint16_t address, uint8_t newValue, bool write);
};