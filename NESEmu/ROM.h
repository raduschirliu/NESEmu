#pragma once

#include <string>

class ROM
{
public:
	ROM(std::string path);
	void read();
	uint8_t getMapperID() const;

	struct Header
	{
		char name[4];
		uint8_t prgSize;
		uint8_t chrSize;
		uint8_t mapperFlags1;
		uint8_t mapperFlags2;
		uint8_t prgRamSize;
		uint8_t tvFlags1;
		uint8_t tvFlags2;
		uint8_t _padding[5];
	} header;
	
	enum Mirroring
	{

	} mirroring;

private:
	uint8_t mapperID;
	std::string path;
};

