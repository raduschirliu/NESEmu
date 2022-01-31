#pragma once

#include "../debug/Logger.h"
#include "Memory.h"

#include <vector>

class PPU
{
public:
	// The start address of each nametable. Each is 960 ($3C0) bytes long
	static constexpr uint16_t NAMETABLE_ADDRESSES[] = { 0x2000, 0x2400, 0x2800, 0x2C00 };
	static constexpr uint16_t NAMETABLE_ROWS = 30;
	static constexpr uint16_t NAMETABLE_COLS = 32;
	static constexpr uint16_t TILE_SIZE = 8;

	// Each attribute table is offset 960 ($3C0) bytes from the start of a nametable
	static constexpr uint16_t ATTRIBUTE_TABLE_OFFSET = 0x3C0;

	// Each attribute table "block" controls 4 tiles in the nametable
	static constexpr uint16_t ATTRIBUTE_TABLE_BLOCK_SIZE = 4;
	static constexpr uint16_t ATTRIBUTE_TABLE_COLS = 8;

	// Amount of entries in OAM
	static constexpr uint16_t OAM_SIZE = 64;

	// Location of registers on CPU memory bus
	static constexpr uint16_t REGISTER_START_ADDRESS = 0x2000;

	// Represents a system palette color
	struct Color
	{
		uint8_t r, g, b, a;
	};

	// Represents a 4 byte sprite stored in the Object Attribute Memory (OAM)
	struct OamSprite
	{
		uint8_t yPos;
		uint8_t tileIndex;
		struct OamSpriteAttributes {
			uint8_t palette : 2;
			uint8_t unused : 3;
			uint8_t priority : 1;
			uint8_t flipHorizontal : 1;
			uint8_t flipVertical : 1;
		} attributes; // Top - LSB, Bottom - MSB
		uint8_t xPos;
	};

	// Represents the PPU registers (0x2000 - 0x2007)
	struct Registers
	{
		struct PpuCtrl {
			uint8_t baseNametable : 2;
			uint8_t addressIncrement : 1;
			uint8_t spritePatternTable : 1;
			uint8_t bgPatternTable : 1;
			uint8_t spriteSize : 1;
			uint8_t masterSlave : 1;
			uint8_t nmiEnable : 1;
		} ctrl; // Top - LSB, Bottom - MSB
		uint8_t mask;
		struct PpuStatus {
			uint8_t lastLsb : 5;
			uint8_t spriteOverflow : 1;
			uint8_t sprite0Hit : 1;
			uint8_t vblank : 1;
		} status; // Top - LSB, Bottom - MSB
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

	// Perform PPU reset
	void reset();

	// Emulate one PPU cycle
	void step();

	// Returns pointer to memory location
	uint8_t *getMemory(uint16_t address);

	// Reads from memory location
	uint8_t readMemory(uint16_t address);

	// Write value in memory location
	void writeMemory(uint16_t address, uint8_t value);

	// Gets current value of PPU registers
	Registers *getRegisters();

	// Returns the OAM sprite at given address ($0 - $255)
	OamSprite *getOamSprite(uint8_t address);

	// Write 256 bytes to Oam
	void writeOamData(uint8_t *data);

	// Lookup the palette index of a nametable entry in the attribute table
	uint8_t getNametableEntryPalette(uint8_t nametable, uint16_t index);

	// Gets the address of the active nametable
	uint16_t getActiveNametableAddress();

	// Gets the address of the background pattern table
	uint16_t getActiveBgPatternTableAddress();

	// Gets the address of the sprite pattern table
	uint16_t getActiveSpritePatternTableAddress();

	// Gets the color palette starting at the given address
	std::vector<Color> getPalette(uint16_t address);

	// Returns the system palette
	std::vector<Color> getSystemPalette();

	// Get the amount of cycles that have occured in the current frame
	uint32_t getCycles();

	// Get the amount of scanlines that have occured
	uint32_t getScanlines();
	
	// Return the total amount of frames that have finished rendering
	uint32_t getFrameCount();

	// Get the total amount of cycles that have occured since reset
	uint32_t getTotalCycles();

	// Returns whether an NMI has occured
	bool getNmiOccured();

	// Returns whether an OAM transfer is requested from the CPU
	bool isOamTransferRequested();

private:
	// Cycle related stats
	uint32_t cycles, scanlines, frames, totalCycles;

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

	// Track when NMI has occured
	bool nmiOccured;

	// Track whether the CPU should transfer data to OAM
	bool oamTransferRequested;

	// If the PPU is currently being reset or not
	bool isResetting;

	// Load the system palette
	void loadPalette(std::string path);

	// Called when one of the PPU's memory mapped registers is accessed
	void onRegisterAccess(uint16_t address, uint8_t newValue, bool write);
};