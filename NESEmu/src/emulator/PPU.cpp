#include "PPU.h"

#include <bitset>
#include <fstream>
#include <iostream>

using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

PPU::PPU(Bus &bus) : logger("..\\logs\\ppu.log"), bus(bus), mapper(nullptr)
{
    // TODO: Convert to modern C++ arrays
    ciram = new uint8_t[CIRAM_SIZE]();
    paletteTables = new uint8_t[PALETTE_TABLE_SIZE]();
    oam = new uint8_t[OAM_SIZE]();

    // Initialize registers
    registers = reinterpret_cast<Registers *>(bus.get(REGISTER_START_ADDRESS));
    registers->ctrl = {0};
    registers->mask = {0};
    registers->status = {0};
    registers->status = {0};
    registers->oamAddr = 0;
    registers->oamData = 0;
    registers->scroll = 0;
    registers->addr = 0;
    registers->data = 0;

    // Load system palette from .pal file
    loadPalette("resources/palette.pal");

    // Set cycle related stats
    cycles = 0;
    scanlines = 0;
    frames = 0;

    // Set access info for PPUADDR/PPUDATA/OAMDMA
    accessAddress = 0;
    accessAddressHighByte = true;
    oamTransferRequested = false;

    // Callbacks
    bus.registerMemoryAccessCallback(
        bind(&PPU::onRegisterAccess, this, _1, _2, _3));
    bus.setPpuOamTransferCallback(bind(&PPU::writeOamData, this, _1));

    reset();
}

PPU::~PPU()
{
    delete[] ciram;
    delete[] paletteTables;
    delete[] oam;
}

void PPU::reset()
{
    registers->ctrl = {0};
    registers->mask = {0};
    registers->scroll = 0;
    registers->addr = 0;

    accessAddress = 0;
    accessAddressHighByte = true;
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
        }
    }

    if (scanlines <= 239)
    {
        // (0 - 239) Rendering, visible scanlines

        if (cycles >= 257 && cycles <= 320)
        {
            registers->oamAddr = 0;
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

        if (cycles >= 257 && cycles <= 320)
        {
            registers->oamAddr = 0;
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

            if (mapper->NametableRead(address, value))
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

        return mapper->ChrRead(address);
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
            if (mapper->NametableWrite(address, value))
            {
                return;
            }

            address = mirrorNametableAddress(address);
            uint16_t ciramOffset = address - 0x2000;

            if (ciramOffset >= CIRAM_SIZE)
            {
                return;
            }

            ciram[ciramOffset] = value;
        }

        mapper->ChrWrite(address, value);
    }
    else if (address >= 0x3F00 && address <= 0x3FFF)
    {
        // $3F00 - $3F1F: Palette RAM
        // $3F20 - $3FFF: Mirrors $3F00 - $3F1F
        uint16_t offset = (address - 0x3F00) % 0x20;
        paletteTables[offset] = value;
    }
}

PPU::Registers *PPU::getRegisters()
{
    return registers;
}

void PPU::writeOamData(uint8_t *data)
{
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
    // Sprite unused attributes always set to 0
    OamSprite *sprite = reinterpret_cast<OamSprite *>(&oam[address]);
    sprite->attributes.unused = 0;
    return sprite;
}

uint8_t PPU::getNametableEntryPalette(uint8_t nametable, uint16_t index)
{
    uint16_t nametableCol = index % NAMETABLE_COLS;
    uint16_t nametableRow = index / NAMETABLE_COLS;
    uint16_t attCol = nametableCol / ATTRIBUTE_TABLE_BLOCK_SIZE;
    uint16_t attRow = nametableRow / ATTRIBUTE_TABLE_BLOCK_SIZE;
    uint16_t attOffset = attRow * ATTRIBUTE_TABLE_COLS + attCol;
    uint16_t address =
        NAMETABLE_ADDRESSES[nametable] + ATTRIBUTE_TABLE_OFFSET + attOffset;
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

std::vector<PPU::Color> PPU::getPalette(uint16_t address)
{
    std::vector<Color> palette;

    if (address == 0x3F00)
    {
        // If only requesting background, return correct color
        uint8_t bgIndex = readMemory(0x3F00);
        palette.push_back(systemPalette[bgIndex]);
    }
    else if (address >= 0x3F01 && address <= 0x3F1F)
    {
        // For other palettes, return transparent color as background
        Color transparent = {0, 0, 0, 0};
        palette.push_back(transparent);

        // Background and sprite palettes
        for (uint16_t i = 0; i < 3; i++)
        {
            uint8_t colorIndex = readMemory(address + i);
            palette.push_back(systemPalette[colorIndex]);
        }
    }

    return palette;
}

std::vector<PPU::Color> PPU::getSystemPalette()
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
    this->mapper = mapper;
}

uint16_t PPU::mirrorNametableAddress(uint16_t address)
{
    switch (mapper->GetMirroringMode())
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

    printf("Invalid mirroring mode: %u\n", mapper->GetMirroringMode());
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
    Color color = {0};
    color.a = 255;

    // Read first 64 palette color entries
    while (!stream.eof() && systemPalette.size() < 64)
    {
        stream.read((char *)&color, 3);
        systemPalette.push_back(color);
    }

    printf("Loaded %zu system palette colors\n", systemPalette.size());
}

void PPU::onRegisterAccess(uint16_t address, uint8_t newValue, bool write)
{
    switch (address)
    {
            // PPUCTRL ($2000)
        case 0x2000:
        {
            // Setting the NMI enable flag during VBlank immediately triggers an
            // NMI
            if (write)
            {
                // TODO: Add this
            }

            break;
        }

        // PPUSTATUS ($2002)
        case 0x2002:
        {
            // Reading status register clears bit 7
            if (!write)
            {
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

        // PPUADDR ($2006)
        case 0x2006:
        {
            if (write)
            {
                uint16_t mask = 0x00FF;
                uint16_t val = newValue;

                if (accessAddressHighByte)
                {
                    mask = 0xFF00;
                    val <<= 8;
                }

                accessAddress &= ~mask;
                accessAddress |= val;

                registers->data = 0;
                accessAddressHighByte = !accessAddressHighByte;

                // If reading palette data, update immediately
                if (accessAddress >= 0x3F00 && accessAddress <= 0x3FFF)
                {
                    registers->data = readMemory(accessAddress);
                }
            }

            break;
        }

        // PPUDATA ($2007)
        case 0x2007:
        {
            if (write)
            {
                writeMemory(accessAddress, newValue);
            }

            // Update value post-read if from memory before the palette data
            if (accessAddress <= 0x3EFF)
            {
                registers->data = readMemory(accessAddress);
            }

            // Increment after access
            if (registers->ctrl.addressIncrement == 0)
            {
                accessAddress++;
            }
            else
            {
                accessAddress += 32;
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