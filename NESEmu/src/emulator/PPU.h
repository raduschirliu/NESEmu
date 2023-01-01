#pragma once

#include "../util/Logger.h"
#include "Bus.h"
#include "Palette.h"
#include "PatternTable.h"

#include <array>
#include <vector>

class PPU
{
public:
	// The start address of each nametable. Each is 960 bytes ($3C0) long
	static constexpr uint16_t NAMETABLE_ADDRESSES[] = { 0x2000, 0x2400, 0x2800, 0x2C00 };
	static constexpr uint16_t NAMETABLE_ROWS = 30;
	static constexpr uint16_t NAMETABLE_COLS = 32;

	// Width of the NES display in pixels
	static constexpr uint16_t SCREEN_WIDTH = NAMETABLE_COLS * PatternTable::TILE_SIZE;
	
	// Height of the NES display in pixels
	static constexpr uint16_t SCREEN_HEIGHT = NAMETABLE_ROWS * PatternTable::TILE_SIZE;

	// Each attribute table is offset 960 ($3C0) bytes from the start of a nametable
	static constexpr uint16_t ATTRIBUTE_TABLE_OFFSET = 0x3C0;

	// Each attribute table "block" controls 4 tiles in the nametable
	static constexpr uint16_t ATTRIBUTE_TABLE_BLOCK_SIZE = 4;

	// Amount of columns in the attribute table
	static constexpr uint16_t ATTRIBUTE_TABLE_COLS = 8;

	// Amount of rows in the attribute table
	static constexpr uint16_t ATTRIBUTE_TABLE_ROWS = 8;

	// Amount of entries in OAM
	static constexpr uint16_t OAM_ENTRIES = 64;

	// Size of OAM in bytes
	static constexpr uint16_t OAM_SIZE = 256;

	// Size of secondary OAM in bytes
	static constexpr uint16_t SECONDARY_OAM_SIZE = 32;

	// Size of palette table in bytes
	static constexpr uint16_t PALETTE_TABLE_SIZE = 0x20;
	
	// Amount of entries in the palette table
	static constexpr uint16_t SYSTEM_PALETTE_ENTRIES = 64;

	// Amount of entries in a single frame palette (3 colors + 1 constant bg)
	static constexpr uint16_t FRAME_PALETTE_ENTRIES = 4;

	// Size of CIRAM, enough to hold two nametables: 2 KiB ($800)
	static constexpr uint16_t CIRAM_SIZE = 0x800;

	// Location of registers on CPU memory bus
	static constexpr uint16_t REGISTER_START_ADDRESS = Bus::PPUCTRL;

	enum class PaletteType : uint8_t
	{
		BACKGROUND = 0,
		SPRITE,

		COUNT
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

	struct InternalRegisters
	{
		struct VramAddress
		{
			uint8_t coarseXScroll : 5; // Controls the current bg tile col (0 - 31)
			uint8_t coarseYScroll : 5; // Controls the current bg tile row (0 - 29)
			uint8_t nametableSelect : 2; // Controls current nametable. bit0 = ntX, bit1 = ntY
			uint8_t fineYScroll : 3;
			uint8_t _padding : 1; // Unused
		}; // Top - LSB, Bottom - MSB

		VramAddress v; // "Loopy V", 15 bits
		VramAddress t; // "Loopy T", 15 bits

		uint8_t x : 3; // Fine X scroll
		uint8_t w : 1; // First/second write toggle bit
	};

	// Represents the memory-mapped PPU registers ($2000 - $2007)
	struct Registers
	{
		struct PpuCtrl { // $2000
			uint8_t baseNametable : 2;
			uint8_t addressIncrement : 1;
			uint8_t spritePatternTable : 1;
			uint8_t bgPatternTable : 1;
			uint8_t spriteSize : 1;
			uint8_t masterSlave : 1;
			uint8_t nmiEnable : 1;
		} ctrl; // Top - LSB, Bottom - MSB
		struct PpuMask { // $2001
			uint8_t grayscale : 1;
			uint8_t showLeftmostBg : 1;
			uint8_t showLeftmostSprite : 1;
			uint8_t showBg : 1;
			uint8_t showSprites : 1;
			uint8_t emphasizeRed : 1;
			uint8_t emphasizeGreen : 1;
			uint8_t emphasizeBlue : 1;
		} mask; // Top - LSB, Bottom - MSB
		struct PpuStatus { // $2002
			uint8_t lastLsb : 5;
			uint8_t spriteOverflow : 1;
			uint8_t sprite0Hit : 1;
			uint8_t vblank : 1;
		} status; // Top - LSB, Bottom - MSB
		uint8_t oamAddr; // $2003
		uint8_t oamData; // $2004
		uint8_t scroll; // $2005
		uint8_t addr; // $2006
		uint8_t data; // $2007
	};

	// Represents a single background tile in the current frame
	struct Tile
	{
		bool valid;
		uint8_t row;
		uint8_t col;
		uint8_t paletteIndex;
		uint8_t patternIndex;
		PatternTable::Pattern patternData; // TODO: Make pointer or ref to avoid copies
	};

	// Represents a single sprite in the current frame
	struct Sprite
	{
		bool valid;
		uint8_t index;
		const OamSprite *oamSprite;
	};

	// Represents a frame the PPU should draw to the screen
	struct Frame
	{
		// TODO: Move init logic?
		Frame() :
			solidBgColor({ 0 }),
			screenPixels(SCREEN_WIDTH * SCREEN_HEIGHT, { 0 }),
			backgroundTiles(NAMETABLE_COLS * NAMETABLE_ROWS, { 0 }),
			sprites(OAM_ENTRIES, { 0 }) {}

		Color solidBgColor;

		// TODO: Implement fixed-size vector util
		std::vector<bool> screenPixels;
		std::vector<PPU::Tile> backgroundTiles;
		std::vector<PPU::Sprite> sprites;
	};

	// Initialize memory
	PPU(Bus &bus);
	
	// Cleanup memory
	~PPU();

	// Perform PPU reset
	void reset();

	// Emulate one PPU cycle
	void step();

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

	// Gets the universal background color
	const Color &getUniversalBgColor();

	// Gets the color palette starting at the given address
	const Palette &getPalette(PaletteType type, uint16_t index);

	// Returns the system palette
	const Palette &getSystemPalette();

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

	// Sets the active mapper
	void setMapper(IMapper *mapper);
	
	// Gets the current frame
	const Frame& getCurrentFrame();

	// Gets a pattern table (0 or 1)
	const PatternTable &getPatternTable(uint8_t index);

private:
	// Cycle related stats
	uint32_t cycles, scanlines, frames, totalCycles;

	// PPU internal VRAM of 2 KiB ($800)
	uint8_t *ciram;

	// Used to hold palette control information, $3F00 - $3FFF. Region $3F20 - $3FFF mirrors $3F00 - $3F1F
	uint8_t *paletteTables;
	
	// Internal Object Attribute Memory (256 bytes)
	uint8_t *oam;
	
	// Secondary OAM memory (32 byte buffer)
	uint8_t *secondaryOam;
	uint8_t secondaryOamIndex;

	// PPU registers
	Registers *registers;
	InternalRegisters internalRegisters;

	// Global system color palette
	Palette systemPalette;

	// TODO: Move into Frame? Rename to "activePalettes"?
	// Color palettes currently active (4 background, 4 sprite) 
	std::array<Palette, 8> framePalettes;

	// Pattern tables
	std::array<PatternTable, 2> patternTables;
	
	// Log PPU activities
	Logger logger;

	// Current frame
	Frame currentFrame;

	// Other NES components
	Bus &bus;
	IMapper *mapper;

	// TODO: These should use internal v/t registers (loopy v/t instead)?
	// Control the address that the CPU can access through PPUADDR/PPUDATA
	uint16_t accessAddress;

	// Keep track of what to fetch during the bg fetching rendering process
	uint16_t bgFetchCounter;

	// Track when NMI has occured
	bool nmiOccured;

	// Track whether the CPU should transfer data to OAM
	bool oamTransferRequested;

	// If the PPU is currently being reset or not
	bool isResetting;

	// Increment coarse X scroll and swap nametable if needed
	void incrementXScroll();

	// Increment fine Y scroll. Overflow to coarse Y or swap nametable if needed
	void incrementYScroll();

	// New frame occured, reset anything that needs to be cleared
	void resetFrame();

	// Fetch the current bg tile in the rendering cycle, and increment V coarse X
	void fetchBgTile();

	// Perform sprite evaluation
	void evaluateSprites();

	// Mirror the nametable address according to the mapper
	uint16_t mirrorNametableAddress(uint16_t address);

	// Load the system palette
	void loadPalette(std::string path);

	// Update the palette tables from what is set in RAM
	void updateFramePalettes();

	// Update pattern data from the pattern tables
	void updatePatternTables();

	// Memory access callback
	void onMemoryAccess(uint16_t address, uint8_t newValue, bool write);
};