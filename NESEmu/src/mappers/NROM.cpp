#include "NROM.h"
#include "../emulator/Bus.h"
#include "../emulator/Cartridge.h"

namespace mappers
{
	NROM::NROM(Cartridge &cartridge) : IMapper(cartridge), cartridge(cartridge), prgRam(PRG_RAM_SIZE)
	{

	}

	uint8_t NROM::getId()
	{
		return 0;
	}

	std::string NROM::getName()
	{
		return "NROM";
	}

	MirroringMode NROM::getMirroringMode()
	{
		// Mirroring pads are soldered based on the cartridge
		uint8_t mirroring = cartridge.getHeader().flags6.mirroring;
		
		if (mirroring)
		{
			return MirroringMode::VERTICAL;
		}

		return MirroringMode::HORIZONTAL;
	}

	// Nametable operations
	bool NROM::nametableRead(uint16_t address, uint8_t &value)
	{
		// Handled by PPU
		return false;
	}

	bool NROM::nametableWrite(uint16_t address, uint8_t value)
	{
		// Handled by PPU
		return false;
	}

	// PRG memory operations
	uint8_t NROM::prgRead(uint16_t address)
	{
		if (address >= 0x6000 && address < 0x8000)
		{
			// PRG RAM, 8 KiB ($2000)
			uint16_t offset = address - 0x6000;
			return prgRam[offset];
		}
		else if (address >= 0x8000 && address <= 0xFFFF)
		{
			// RPG ROM, either 16 KiB ($4000) mirrored or 32 KiB ($8000)
			uint16_t offset = (address - 0x8000) % cartridge.getPrgRom().size();
			return cartridge.getPrgRom()[offset];
		}

		return 0;
	}

	void NROM::prgWrite(uint16_t address, uint8_t value)
	{
		if (address >= 0x6000 && address < 0x8000)
		{
			// PRG RAM, 8 KiB ($2000)
			uint16_t offset = address - 0x6000;
			prgRam[offset] = value;
		}
		else if (address >= 0x8000 && address <= 0xFFFF)
		{
			// RPG ROM, either 16 KiB ($4000) mirrored or 32 KiB ($8000)
			// TODO: Determine why it needs to write to the ROM?
			uint16_t offset = (address - 0x8000) % cartridge.getPrgRom().size();
			cartridge.getPrgRom()[offset] = value;
		}
	}

	// CHR memory operations
	uint8_t NROM::chrRead(uint16_t address)
	{
		// NROM allows for max 8 KiB ($2000) of CHR ROM
		if (address < 0x2000)
		{
			return cartridge.getChrRom()[address];
		}

		return 0;
	}

	void NROM::chrWrite(uint16_t address, uint8_t value)
	{
		// CHR ROM is read only
	}
}