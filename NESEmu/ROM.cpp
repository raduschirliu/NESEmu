#include "ROM.h"
#include <fstream>

ROM::ROM(std::string path)
{
	this->path = path;
	mapperID = 0;
}

void ROM::read()
{
	std::ifstream stream;
	stream.open(path, std::ifstream::binary);

	if (stream.is_open())
	{
		// Read header
		stream.get((char *)&header, sizeof(Header));

		// Check if trainer (512 bytes) is present
		if (header.mapperFlags1 & 0b100)
		{
			stream.seekg(512, std::ios_base::cur);
		}

		// Set mapper ID
		mapperID = header.mapperFlags2 & 0xF;
		mapperID |= (header.mapperFlags1 & 0xF) >> 4;
	}
}

uint8_t ROM::getMapperID() const
{
	return mapperID;
}