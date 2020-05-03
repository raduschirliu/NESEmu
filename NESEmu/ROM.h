#pragma once

#include <string>

class ROM
{
public:
	struct Header
	{
		char name[4];
		uint8_t prgBanks;
		uint8_t chrBanks;
		uint8_t mapperFlags1;
		uint8_t mapperFlags2;
		uint8_t prgRamSize;
		uint8_t tvFlags1;
		uint8_t tvFlags2;
		uint8_t _padding[5];
	} header;

	enum class Mirroring : uint8_t
	{
		Horizontal = 0,
		Vertical
	} mirroring;

	ROM(std::string path);
	void read();
	uint8_t getMapperID() const;

private:
	uint8_t mapperID;
	std::string path;
};

