#include "emulator/mappers/NROM.h"

#include "emulator/Bus.h"
#include "emulator/Cartridge.h"

// 8 KiB ($2000) of PRG RAM provided (instead of the original 2 KiB on the
// NES)
static constexpr uint16_t kPrgRamSize = 0x2000;

namespace mappers
{

NROM::NROM(Cartridge &cartridge)
    : IMapper(cartridge),
      cartridge_(cartridge),
      prg_ram_(kPrgRamSize)
{
}

uint8_t NROM::GetId()
{
    return 0;
}

std::string NROM::GetName()
{
    return "NROM";
}

MirroringMode NROM::GetMirroringMode()
{
    // Mirroring pads are soldered based on the cartridge
    uint8_t mirroring = cartridge_.GetHeader().flags6.mirroring;

    if (mirroring)
    {
        return MirroringMode::kVertical;
    }

    return MirroringMode::kHorizontal;
}

// Nametable operations
bool NROM::NametableRead(uint16_t address, uint8_t &value)
{
    // Handled by PPU
    return false;
}

bool NROM::NametableWrite(uint16_t address, uint8_t value)
{
    // Handled by PPU
    return false;
}

// PRG memory operations
uint8_t NROM::PrgRead(uint16_t address)
{
    if (address >= 0x6000 && address < 0x8000)
    {
        // PRG RAM, 8 KiB ($2000)
        uint16_t offset = address - 0x6000;
        return prg_ram_[offset];
    }
    else if (address >= 0x8000 && address <= 0xFFFF)
    {
        // RPG ROM, either 16 KiB ($4000) mirrored or 32 KiB ($8000)
        uint16_t offset = (address - 0x8000) % cartridge_.GetPrgRom().size();
        return cartridge_.GetPrgRom()[offset];
    }

    return 0;
}

void NROM::PrgWrite(uint16_t address, uint8_t value)
{
    if (address >= 0x6000 && address < 0x8000)
    {
        // PRG RAM, 8 KiB ($2000)
        uint16_t offset = address - 0x6000;
        prg_ram_[offset] = value;
    }
    else if (address >= 0x8000 && address <= 0xFFFF)
    {
        // RPG ROM, either 16 KiB ($4000) mirrored or 32 KiB ($8000)
        // TODO: Determine why it needs to write to the ROM?
        uint16_t offset = (address - 0x8000) % cartridge_.GetPrgRom().size();
        cartridge_.GetPrgRom()[offset] = value;
    }
}

// CHR memory operations
uint8_t NROM::ChrRead(uint16_t address)
{
    // NROM allows for max 8 KiB ($2000) of CHR ROM
    if (address < 0x2000)
    {
        return cartridge_.GetChrRom()[address];
    }

    return 0;
}

void NROM::ChrWrite(uint16_t address, uint8_t value)
{
    // CHR ROM is read only
}

}  // namespace mappers