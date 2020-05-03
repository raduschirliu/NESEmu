#pragma once

#include <cstdint>

class Mapper
{
public:
	Mapper();
	~Mapper();

private:
	uint8_t prgBanks;
	uint8_t chrBanks;
};

