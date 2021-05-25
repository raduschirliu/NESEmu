#include "ROM.h"
#include <fstream>
#include <cerrno>

ROM::ROM(std::string path)
{
	this->path = path;
	mapperID = 0;
}

void ROM::map(Memory *memory, PPU *ppu)
{
	if (memory == nullptr || ppu == nullptr)
	{
		printf("Invalid memory pointer provided to main Memory or PPU\n");
		return;
	}

	std::ifstream stream;
	stream.open(path, std::ifstream::binary);

	if (stream.is_open())
	{
		// Read header
		stream.get((char *)&header, sizeof(Header));

		// Check if trainer is present
		if (header.mapperFlags1 & 0b100)
		{
			// Skip trainer if it is present (512 bytes)
			stream.seekg(512, std::ios_base::cur);
		}

		// Set mapper ID
		mapperID = header.mapperFlags2 & 0xF;
		mapperID |= (header.mapperFlags1 & 0xF) >> 4;

		// Set mirroring
		mirroring = (Mirroring)(header.mapperFlags1 & 0x1);

		// TODO: Implement mappers
		// TEST: Write 0x4000 bytes (16KB) to memory, mirrored at 0x8000-0xBFFF and 0xC000-0xFFFF
		// Equivalent to using mapper 0
		uint16_t offset = 0;
		uint8_t byte;

		stream.seekg(1, std::ios_base::cur);

		while (!stream.eof() && offset < 0x4000)
		{
			stream.read((char *)&byte, 1);
			memory->set(0x8000 + offset, byte);
			memory->set(0xC000 + offset, byte);

			offset++;
		}

		printf("Done, loaded %d bytes in memory locations:\n", offset);
		printf("\t%X - %X\n", 0x8000, 0x8000 + offset - 1);
		printf("\t%X - %X\n\n", 0xC000, 0xC000 + offset - 1);
	}
	else
	{
		printf("Failed to open file\n");
	}
}

uint8_t ROM::getMapperID() const
{
	return mapperID;
}