#pragma once

#include "../debug/Logger.h"

// TODO: Implement NES PPU
class PPU
{
public:
	// Represents a 4 byte sprite stored in the Object Attribute Memory (OAM)
	struct OamSprite
	{
		uint8_t yPos;
		uint8_t tileIndex;
		uint8_t attributes;
		uint8_t xPos;
	};

	// Initialize memory
	PPU();
	
	// Cleanup memory
	~PPU();

	// Returns pointer to memory location
	uint8_t *getMemory(uint16_t address);

	// Reads from memory location
	uint8_t readMemory(uint16_t address);

	// Sets value in memory location
	void setMemory(uint16_t address, uint8_t value);

private:
	// Used for pattern tables, $0000 - $1FFF. Each pattern table being $1000 in size
	// Mapped to cartridge using bank switching
	uint8_t *patternTables;

	// Used for name tables, $2000 - $2FFF. Normally mapped to 2kB of internal NES VRAM
	uint8_t *nameTables;

	// Used to hold palette control information, $3F00 - $3FFF. Region $3F20 - $3FFF mirrors $3F00 - $3F1F
	uint8_t *paletteTables;
	
	// Internal Object Attribute Memory (256kB)
	uint8_t *oam;
	
	// Log PPU activities
	Logger logger;
};