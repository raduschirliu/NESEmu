#include "ROM.h"
#include <fstream>
#include <cerrno>

ROM::ROM()
{
	path = "";
	mapperID = 0;
}

ROM::ROM(std::string path)
{
	this->path = path;
	mapperID = 0;
}

void ROM::load(std::string path)
{
	this->path = path;
	mapperID = 0;
}

void ROM::map(Memory &memory, PPU &ppu)
{
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
				memory.set(0x8000 + offset, byte);
				memory.set(0xC000 + offset, byte);
			}

			// Copy next 8KB of CHR ROM to PPU memory
			if (offset >= 0x4000 && offset < 0x6000)
			{
				ppu.setMemory(offset - 0x4000, byte);
			}

			offset++;
		}

		printf("Done, loaded %X (%d) bytes from ROM\n", offset, offset);
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

std::string ROM::getPath() const
{
	return path;
}