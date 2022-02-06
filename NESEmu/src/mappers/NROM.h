#pragma once

#include "../emulator/IMapper.h"

#include <vector>

namespace mappers
{
	// Mapper 0: NROM
	class NROM : public IMapper
	{
	public:
		// 8 KiB ($2000) of PRG RAM provided (instead of the original 2 KiB on the NES)
		static constexpr uint16_t PRG_RAM_SIZE = 0x2000;

		NROM(Cartridge &cartridge);
		uint8_t getId() override;
		std::string getName() override;
		MirroringMode getMirroringMode() override;

		// Return true if the cartridge will handle the read/write operation
		bool nametableRead(uint16_t address, uint8_t &value) override;
		bool nametableWrite(uint16_t address, uint8_t value) override;

		// PRG memory operations
		uint8_t prgRead(uint16_t address) override;
		void prgWrite(uint16_t address, uint8_t value) override;

		// CHR memory operations
		uint8_t chrRead(uint16_t address) override;
		void chrWrite(uint16_t address, uint8_t value) override;

	private:
		Cartridge &cartridge;
		std::vector<uint8_t> prgRam;
	};
}
