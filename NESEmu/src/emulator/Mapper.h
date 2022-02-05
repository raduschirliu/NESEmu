#pragma once

#include <cstdint>
#include <unordered_map>

// TODO
// Base class for mapper representation
class Mapper
{
public:
	Mapper(uint8_t prgBanks, uint8_t chrBanks);

private:
	uint8_t prgBanks;
	uint8_t chrBanks;
};

