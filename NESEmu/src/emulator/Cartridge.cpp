#include "Cartridge.h"
#include <fstream>
#include <cerrno>

Cartridge::Cartridge() : header({ 0 }), path(""), mapper(nullptr)
{

}

void Cartridge::load(std::string path)
{
	this->path = path;
}

void Cartridge::map(Bus &bus, CPU &cpu, PPU &ppu)
{
	std::ifstream stream;
	stream.open(path, std::ifstream::binary);

	if (!stream.is_open())
	{
		printf("Failed to open file\n");
		return;
	}

	// Read header
	stream.get((char *)&header, sizeof(Header));

	if (memcmp(header.name, HEADER_NAME, sizeof(HEADER_NAME)) != 0)
	{
		printf("Error, cartridge loaded rom with invalid header constant\n");
		return;
	}

	// Check if trainer is present
	if (header.flags6.hasTrainer)
	{
		stream.seekg(TRAINER_SIZE, std::ios_base::cur);
	}

	// TODO: Implement mappers
	// Equivalent to using mapper 0 (NROM)
	uint16_t offset = 0;
	uint8_t byte;

	stream.seekg(1, std::ios_base::cur);

	while (!stream.eof())
	{
		stream.read((char *)&byte, 1);
			
		// Copy first 16KB of PRG ROM to CPU memory
		if (offset >= 0 && offset < 0x4000)
		{
			// TEST: Write 0x4000 bytes (16KB) to memory, mirrored at 0x8000-0xBFFF and 0xC000-0xFFFF
			bus.write(0x8000 + offset, byte);
			bus.write(0xC000 + offset, byte);
		}

		// Copy next 8KB of CHR ROM to PPU memory
		if (offset >= 0x4000 && offset < 0x6000)
		{
			ppu.writeMemory(offset - 0x4000, byte);
		}

		offset++;
	}

	printf("Done, loaded $%X (%d) bytes from ROM\n", offset, offset);
}

Cartridge::Header Cartridge::getHeader() const
{
	return header;
}

uint8_t Cartridge::getMapperID() const
{
	return header.flags6.mapperLowerNibble & (header.flags7.mapperUpperNibble << 4);
}

std::string Cartridge::getPath() const
{
	return path;
}