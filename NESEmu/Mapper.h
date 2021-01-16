#pragma once

#include <cstdint>

// TODO
// Base class for mapper representation
class Mapper
{
public:
	Mapper();
	~Mapper();

private:
	uint8_t prgBanks;
	uint8_t chrBanks;
};

