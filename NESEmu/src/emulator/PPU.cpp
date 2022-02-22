#include "PPU.h"

#include <fstream>
#include <iostream>
#include <assert.h>

using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

PPU::PPU(Bus &bus) : logger("..\\logs\\ppu.log"), bus(bus), mapper(nullptr), systemPalette(SYSTEM_PALETTE_ENTRIES)
{
	// TODO: Convert to modern C++ arrays
	ciram = new uint8_t[CIRAM_SIZE]();
	paletteTables = new uint8_t[PALETTE_TABLE_SIZE]();
	oam = new uint8_t[OAM_SIZE]();
	secondaryOam = new uint8_t[SECONDARY_OAM_SIZE]();

	assert(ciram);
	assert(paletteTables);
	assert(oam);
	assert(secondaryOam);

	// Initialize registers
	internalRegisters = { 0 };

	registers = reinterpret_cast<Registers *>(bus.get(REGISTER_START_ADDRESS));
	registers->ctrl = { 0 };
	registers->mask = { 0 };
	registers->status = { 0 };
	registers->status = { 0 };
	registers->oamAddr = 0;
	registers->oamData = 0;
	registers->scroll = 0;
	registers->addr = 0;
	registers->data = 0;

	// Load system palette from .pal file
	loadPalette("palette.pal");

	// Set cycle related stats
	cycles = 0;
	scanlines = 0;
	frames = 0;

	// Set access info for PPUADDR/PPUDATA/OAMDMA
	accessAddress = 0;
	oamTransferRequested = false;
	bgFetchCounter = 0;
	nmiOccured = false;

	// Callbacks
	bus.registerMemoryAccessCallback(bind(&PPU::onMemoryAccess, this, _1, _2, _3));
	bus.setPpuOamTransferCallback(bind(&PPU::writeOamData, this, _1));

	reset();
}

PPU::~PPU()
{
	assert(ciram);
	assert(paletteTables);
	assert(oam);
	assert(secondaryOam);

	delete[] ciram;
	delete[] paletteTables;
	delete[] oam;
	delete[] secondaryOam;
}

void PPU::reset()
{
	registers->ctrl = { 0 };
	registers->mask = { 0 };
	registers->scroll = 0;
	registers->addr = 0;

	// TODO: Resetting internal registers?
	internalRegisters = { 0 };

	accessAddress = 0;
	isResetting = true;
}

void PPU::step()
{
	// While resetting, registers should not be able to be set
	if (isResetting)
	{
		reset();
	}

	// 341 PPU cycles per scanline (0 - 340)
	if (cycles >= 341)
	{
		cycles -= 341;
		scanlines++;

		// 261 scanlines per frame
		if (scanlines > 261)
		{
			scanlines = 0;
			frames++;
			resetFrame();
		}
	}

	if (scanlines <= 239)
	{
		// (0 - 239) Rendering, visible scanlines

		if (cycles > 0 && cycles < 257 && registers->mask.showBg)
		{
			fetchBgTile();

			if (cycles == 256)
			{
				incrementYScroll();
			}
		}
		else if (cycles >= 257 && cycles < 321)
		{
			// Idle cycles to reset registers, no rendering/fetches
			registers->oamAddr = 0;

			if (cycles == 257 && registers->mask.showBg)
			{
				// Copy all bits related to horizontal position from t -> v
				internalRegisters.v.coarseXScroll = internalRegisters.t.coarseXScroll;

				internalRegisters.v.nametableSelect &= 0b10;
				internalRegisters.v.nametableSelect |= (internalRegisters.t.nametableSelect & 0b01);
			}
		}
		else if (cycles >= 321 && cycles < 337)
		{
			// TODO: First two tiles on next scanline
		}
		else
		{
			// Unused nametable fetches
		}

		if (registers->mask.showSprites)
		{
			evaluateSprites();
		}
	}
	else if (scanlines == 240)
	{
		// (240) Post-render scan line - PPU idles
	}
	else if (scanlines >= 241 && scanlines <= 260)
	{
		// (241 - 260) Vblank

		if (scanlines == 241 && cycles == 1)
		{
			// Set VBlank and trigger NMI
			registers->status.vblank = 1;
			nmiOccured = true;

			if (registers->ctrl.nmiEnable)
			{
				bus.dispatchNmi();
			}
		}
	}
	else if (scanlines >= 261)
	{
		// (261) Pre-render scanline

		if (cycles == 1)
		{
			// Clear VBlank
			registers->status.vblank = 0;
			nmiOccured = false;

			// Clear sprite flags
			registers->status.sprite0Hit = 0;
			registers->status.spriteOverflow = 0;

			// Clear resetting status
			isResetting = false;
		}

		if (cycles == 256 && registers->mask.showBg)
		{
			incrementYScroll();
		}

		if (cycles >= 257 && cycles <= 320)
		{
			registers->oamAddr = 0;
		}

		if (cycles == 257 && registers->mask.showBg)
		{
			// Copy all bits related to horizontal position from t -> v
			internalRegisters.v.coarseXScroll = internalRegisters.t.coarseXScroll;

			internalRegisters.v.nametableSelect &= 0b10;
			internalRegisters.v.nametableSelect |= (internalRegisters.t.nametableSelect & 0b01);
		}

		if (cycles >= 280 && cycles <= 304 && registers->mask.showBg)
		{
			// Copy all vertical position bits from t -> v
			internalRegisters.v.coarseYScroll = internalRegisters.t.coarseYScroll;
			internalRegisters.v.fineYScroll = internalRegisters.t.fineYScroll;

			internalRegisters.v.nametableSelect &= 0b01;
			internalRegisters.v.nametableSelect |= (internalRegisters.t.nametableSelect & 0b10);
		}

		if (registers->mask.showSprites)
		{
			evaluateSprites();
		}
	}

	cycles++;
	totalCycles++;
}

uint8_t PPU::readMemory(uint16_t address)
{
	if (address <= 0x3EFF)
	{
		// $0000 - $1FFF: Pattern tables
		// $2000 - $2FFF: Nametables
		// $3000 - $3EFF: Mirrors $2000 - $2EFF

		if (address >= 0x3000)
		{
			address -= 0x1000;
		}

		if (address >= 0x2000 && address <= 0x2FFF)
		{
			uint8_t value;

			if (mapper->nametableRead(address, value))
			{
				return value;
			}

			address = mirrorNametableAddress(address);
			uint16_t ciramOffset = address - 0x2000;

			if (ciramOffset >= CIRAM_SIZE)
			{
				return 0;
			}

			return ciram[ciramOffset];
		}

		return mapper->chrRead(address);
	}
	else if (address >= 0x3F00 && address <= 0x3FFF)
	{
		// $3F00 - $3F1F: Palette RAM
		// $3F20 - $3FFF: Mirrors $3F00 - $3F1F
		uint16_t offset = (address - 0x3F00) % 0x20;
		return paletteTables[offset];
	}

	return 0;
}

void PPU::writeMemory(uint16_t address, uint8_t value)
{
	if (address <= 0x3EFF)
	{
		// $0000 - $1FFF: Pattern tables
		// $2000 - $2FFF: Nametables
		// $3000 - $3EFF: Mirrors $2000 - $2EFF

		if (address >= 0x3000)
		{
			address -= 0x1000;
		}

		if (address >= 0x2000 && address <= 0x2FFF)
		{
			if (mapper->nametableWrite(address, value))
			{
				return;
			}

			address = mirrorNametableAddress(address);
			uint16_t ciramOffset = address - 0x2000;

			if (ciramOffset >= CIRAM_SIZE)
			{
				printf("-------> OOB array: Wrote $%X = $%X\n", address, value);
				return;
			}

			// printf("Wrote $%X = $%X\n", address, value);
			ciram[ciramOffset] = value;
		}
		else
		{
			printf("-------> OOB pattern: Wrote $%X = $%X\n", address, value);
		}

		mapper->chrWrite(address, value);
	}
	else if (address >= 0x3F00 && address <= 0x3FFF)
	{
		// $3F00 - $3F1F: Palette RAM
		// $3F20 - $3FFF: Mirrors $3F00 - $3F1F
		uint16_t offset = (address - 0x3F00) % 0x20;
		paletteTables[offset] = value;
		updateFramePalettes();
	}
}

PPU::Registers *PPU::getRegisters()
{
	return registers;
}

void PPU::writeOamData(uint8_t *data)
{
	assert(data);
	int spriteByte = 0;

	for (int i = 0; i < 256; i++)
	{
		oam[i] = data[i];

		// Always set unused bytes of sprite to 0
		if (spriteByte == 2)
		{
			oam[i] &= 0b11100011;
		}

		spriteByte++;

		if (spriteByte >= 4)
		{
			spriteByte = 0;
		}
	}

	registers->oamData = oam[registers->oamAddr];
	oamTransferRequested = false;
}

PPU::OamSprite *PPU::getOamSprite(uint8_t address)
{
	assert(address < OAM_SIZE);

	// Sprite unused attributes always set to 0
	OamSprite *sprite = reinterpret_cast<OamSprite *>(&oam[address]);
	sprite->attributes.unused = 0;
	return sprite;
}

uint8_t PPU::getNametableEntryPalette(uint8_t nametable, uint16_t index)
{
	assert(nametable < 4);
	assert(index < NAMETABLE_COLS * NAMETABLE_ROWS);

	uint16_t nametableCol = index % NAMETABLE_COLS; 
	uint16_t nametableRow = index / NAMETABLE_COLS;
	uint16_t attCol = nametableCol / ATTRIBUTE_TABLE_BLOCK_SIZE;
	uint16_t attRow = nametableRow / ATTRIBUTE_TABLE_BLOCK_SIZE;
	uint16_t attOffset = attRow * ATTRIBUTE_TABLE_COLS + attCol;
	uint16_t address = NAMETABLE_ADDRESSES[nametable] + ATTRIBUTE_TABLE_OFFSET + attOffset;
	uint8_t byte = readMemory(address);
	uint8_t shift = 0;

	if (nametableCol % 4 >= 2)
	{
		// Right half of block
		shift += 2;
	}

	if (nametableRow % 4 >= 2)
	{
		// Bottom half of block
		shift += 4;
	}

	return (byte & (0b11 << shift)) >> shift;
}

uint16_t PPU::getActiveNametableAddress()
{
	return NAMETABLE_ADDRESSES[registers->ctrl.baseNametable];
}

uint16_t PPU::getActiveBgPatternTableAddress()
{
	// One of: 0x0000, 0x1000
	return registers->ctrl.bgPatternTable * 0x1000;
}

uint16_t PPU::getActiveSpritePatternTableAddress()
{
	// One of: 0x0000, 0x1000
	return registers->ctrl.spritePatternTable * 0x1000;
}

const Color& PPU::getUniversalBgColor()
{
	return framePalettes[0].getColors()[0];
}

const Palette &PPU::getPalette(PaletteType type, uint16_t index)
{
	assert(type < PaletteType::COUNT);
	assert(index < framePalettes.size());

	if (type == PaletteType::SPRITE)
	{
		index += 4;
	}

	return framePalettes[index];
}

const Palette& PPU::getSystemPalette()
{
	return systemPalette;
}

bool PPU::getNmiOccured()
{
	// NMI only occurs during VBlank if NMI enable flag of PpuCtrl is set
	return nmiOccured && registers->ctrl.nmiEnable;
}

bool PPU::isOamTransferRequested()
{
	return oamTransferRequested;
}

void PPU::setMapper(IMapper *mapper)
{
	assert(mapper);
	this->mapper = mapper;
	updatePatternTables();
}

const PPU::Frame& PPU::getCurrentFrame()
{
	return currentFrame;
}

void PPU::incrementXScroll()
{
	if (internalRegisters.v.coarseXScroll == 31)
	{
		// Last column, switch horizontal nametable
		internalRegisters.v.nametableSelect ^= 0b01;
	}

	internalRegisters.v.coarseXScroll++;
}

void PPU::incrementYScroll()
{
	if (internalRegisters.v.fineYScroll < 7)
	{
		internalRegisters.v.fineYScroll++;
	}
	else
	{
		// Wrap fine Y
		internalRegisters.v.fineYScroll = 0;

		if (internalRegisters.v.coarseYScroll == 29)
		{
			// Last row of nametable, switch vertical nametable
			internalRegisters.v.coarseYScroll = 0;
			internalRegisters.v.nametableSelect ^= 0b10;
		}
		else
		{
			internalRegisters.v.coarseYScroll++;
		}
	}
}

void PPU::resetFrame()
{
	// Set tiles as "invalid"
	for (auto &tile : currentFrame.backgroundTiles)
	{
		tile.valid = false;
	}
}

void PPU::fetchBgTile()
{
	uint16_t tileIndex = 
		static_cast<uint16_t>(internalRegisters.v.coarseYScroll) * NAMETABLE_COLS +
		static_cast<uint16_t>(internalRegisters.v.coarseXScroll);
	Tile &tile = currentFrame.backgroundTiles[tileIndex];

	// Takes 8 cycles to fetch a bg tile
	if (!tile.valid)
	{
		if (bgFetchCounter == 0)
		{
			tile.col = internalRegisters.v.coarseXScroll;
			tile.row = internalRegisters.v.coarseYScroll;
		}
		else if (bgFetchCounter == 1)
		{
			// Nametable byte fetch
			uint16_t nametableAddress = NAMETABLE_ADDRESSES[internalRegisters.v.nametableSelect];
			uint16_t address = nametableAddress + tile.row * NAMETABLE_COLS + tile.col;

			tile.patternIndex = readMemory(address);
		}
		else if (bgFetchCounter == 3)
		{
			// Attribute table byte fetch
			tile.paletteIndex = getNametableEntryPalette(internalRegisters.v.nametableSelect, tileIndex);
		}
		else if (bgFetchCounter == 7)
		{
			// Pattern table byte fetch
			// TODO: Move pattern table fetch logic from table into here
			tile.patternData = patternTables[registers->ctrl.baseNametable].getPattern(tile.patternIndex);
			tile.valid = true;
		}
	}

	bgFetchCounter++;

	if (bgFetchCounter > 7)
	{
		bgFetchCounter = 0;
		incrementXScroll();
	}
}

void PPU::evaluateSprites()
{
	uint16_t nextScanline = scanlines + 1;

	if (scanlines == 261)
	{
		// Pre-render scanline
		nextScanline = 0;
	}

	if (cycles >= 1 && cycles <= 64)
	{
		// Clear secondary OAM
		memset(secondaryOam, SECONDARY_OAM_SIZE, 0xFF);
		registers->oamData = 0xFF;
	}
	else if (cycles >= 65 && cycles <= 256)
	{
		// Sprite evaluation
	}
	else if (cycles >= 257 && cycles <= 320)
	{
		// Sprite fetches
	}
	else if (cycles == 0 || (cycles >= 321 && cycles <= 340))
	{
		// Background render pipeline init

	}
}

uint16_t PPU::mirrorNametableAddress(uint16_t address)
{
	switch (mapper->getMirroringMode())
	{
	case MirroringMode::HORIZONTAL:
	{
		if ((address >= 0x2400 && address < 0x2800) ||
			(address >= 0x2C00 && address < 0x3000))
		{
			// Each nametable is 1024 bytes
			address -= 1024;
		}

		return address;
		break;
	}

	case MirroringMode::VERTICAL:
	{
		if (address >= 0x2800)
		{
			// Each nametable is 1024 bytes
			address -= 1024 * 2;
		}

		return address;
		break;
	}

	// TODO: Add other mirroring options
	}

	printf("Invalid mirroring mode: %u\n", mapper->getMirroringMode());
	return 0;
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

	// NES colors have no alpha value, so set to max
	std::vector<Color> colors;
	colors.reserve(SYSTEM_PALETTE_ENTRIES);

	// Read first 64 palette color entries
	for (size_t i = 0; i < SYSTEM_PALETTE_ENTRIES; i++)
	{
		Color color;
		stream.read((char *)&color, 3);
		color.a = 255;
		colors.push_back(color);
	}

	systemPalette.setColors(colors);
	printf("Loaded %zu system palette colors\n", colors.size());
}

void PPU::updateFramePalettes()
{
	uint16_t paletteOffsets[] = { 0x01, 0x05, 0x09, 0x0D, 0x11, 0x15, 0x19, 0x1D };
	auto& systemColors = systemPalette.getColors();
	Color bgColor = systemColors[paletteTables[0]];

	currentFrame.solidBgColor = bgColor;

	for (size_t i = 0; i < 8; i++)
	{
		std::vector<Color> colors;
		colors.reserve(4);
		colors.push_back(bgColor);

		for (size_t colorIdx = 0; colorIdx < 3; colorIdx++)
		{
			colors.push_back(systemColors[paletteTables[paletteOffsets[i] + colorIdx]]);
		}

		framePalettes[i].setColors(colors);
	}
}

void PPU::updatePatternTables()
{
	patternTables[0].load(*this, 0x0000);
	patternTables[1].load(*this, 0x1000);
}

void PPU::onMemoryAccess(uint16_t address, uint8_t newValue, bool write)
{
	// Memory mapped registers
	// v,t,x,y == internal registers; d = newValue
	switch (address)
	{
		// PPUCTRL ($2000)
	case 0x2000:
	{
		if (write)
		{
			// TODO: Setting the NMI enable flag during VBlank immediately triggers an NMI

			// t: ...GH.. ........ <- d: ......GH
			internalRegisters.t.nametableSelect = registers->ctrl.baseNametable;
		}

		break;
	}

	// PPUSTATUS ($2002)
	case 0x2002:
	{
		// Reading status register clears vblank flag and write toggle
		if (!write)
		{
			internalRegisters.w = 0;
			registers->status.vblank = 0;
			nmiOccured = false;
		}

		break;
	}

	// OAMDATA ($2004)
	case 0x2004:
	{
		if (write)
		{
			oam[registers->oamAddr] = newValue;
			registers->oamAddr++;
			registers->oamData = oam[registers->oamAddr];
		}

		break;
	}

	// PPUSCROLL $(2005)
	case 0x2005:
	{
		if (write)
		{
			if (internalRegisters.w == 0)
			{
				/*
					First write
					t: ....... ...ABCDE <- d: ABCDE...
					x:              FGH <- d: .....FGH
					w:                  <- 1
				*/
				internalRegisters.t.coarseXScroll = newValue & 0b11111000;
				internalRegisters.x = newValue & 0b111;
				internalRegisters.w = 1;
			}
			else
			{
				/*
					Second write
					t: FGH..AB CDE..... <- d: ABCDEFGH
					w:                  <- 0
				*/
				internalRegisters.t.coarseYScroll = newValue & 0b11111000;
				internalRegisters.t.fineYScroll = newValue & 0b111;
				internalRegisters.w = 0;
			}
		}

		break;
	}

	// PPUADDR ($2006)
	case 0x2006:
	{
		if (write)
		{
			uint16_t *t = reinterpret_cast<uint16_t *>(&internalRegisters.t);

			if (internalRegisters.w == 0)
			{
				/*
					First write
					t: .CDEFGH ........ <- d: ..CDEFGH
					<unused>     <- d: AB......
					t: Z...... ........ <- 0 (bit Z is cleared)
					w:                  <- 1
				*/
				(*t) &= 0x00FF;
				(*t) |= ((newValue & 0b00111111) << 8);
				internalRegisters.w = 1;
			}
			else
			{
				/*
					Second write
					t: ....... ABCDEFGH <- d: ABCDEFGH
					v: <...all bits...> <- t: <...all bits...>
					w:                  <- 0
				*/
				(*t) &= 0xFF00;
				(*t) |= newValue;
				internalRegisters.v = internalRegisters.t;
				internalRegisters.w = 0;

				// If reading palette data, update immediately
				if (*t >= 0x3F00 && *t <= 0x3FFF)
				{
					registers->data = readMemory(*t);
				}
			}
		}

		break;
	}

	// PPUDATA ($2007)
	case 0x2007:
	{
		// Write address == internalRegisters.v
		uint16_t *address = reinterpret_cast<uint16_t *>(&internalRegisters.v);

		if (write)
		{
			writeMemory(*address, newValue);
		}

		// Update value post-read if from memory before the palette data
		if (*address <= 0x3EFF)
		{
			registers->data = readMemory(*address);
		}

		// Increment after access
		if (registers->ctrl.addressIncrement == 0)
		{
			(*address)++;
		}
		else
		{
			*address += 32;
		}

		break;
	}
	}
}

uint32_t PPU::getCycles()
{
	return cycles;
}

uint32_t PPU::getScanlines()
{
	return scanlines;
}

uint32_t PPU::getFrameCount()
{
	return frames;
}

uint32_t PPU::getTotalCycles()
{
	return totalCycles;
}

const PatternTable &PPU::getPatternTable(uint8_t index)
{
	assert(index < patternTables.size());
	return patternTables[index];
}
