#include "ROM.h"
#include <fstream>
#include <cerrno>

ROM::ROM(std::string path)
{
	this->path = path;
	mapperID = 0;
}

void ROM::read(Memory *memory)
{
	if (memory == nullptr)
	{
		printf("Invalid memory pointer provided to ROM\n");
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

		// TEST: write all data to memory
		uint16_t offset = 0;
		uint8_t byte;

		stream.seekg(1, std::ios_base::cur);

		while (!stream.eof() && offset < 0x4000)
		{
			stream.read((char *)&byte, 1);
			//memory->set(0x8000 + offset, byte);
			memory->set(0xC000 + offset, byte);

			//printf("%X -> %X\n", 0xC000 + offset, byte);
			offset++;
		}

		printf("%X, %X\n", 0x8000 + offset, 0xC000 + offset);
		printf("Done, loaded %d bytes\n\n", offset);
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