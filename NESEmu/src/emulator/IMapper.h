#pragma once

#include "MirroringMode.h"

#include <cstdint>
#include <unordered_map>
#include <string>

// Forward declaration
class Cartridge;

// Base interface for mapper representation
class IMapper
{
public:
	IMapper(Cartridge &cartridge) { }
	virtual uint8_t getNumber() = 0;
	virtual std::string getName() = 0;
	virtual MirroringMode getMirroringMode() = 0;

	// Return true if the cartridge will handle the read/write operation
	virtual bool nametableRead(uint16_t address, uint8_t &value) = 0;
	virtual bool nametableWrite(uint16_t address, uint8_t value) = 0;

	// PRG memory operations
	virtual uint8_t prgRead(uint16_t address) = 0;
	virtual void prgWrite(uint16_t address, uint8_t value) = 0;

	// CHR memory operations
	virtual uint8_t chrRead(uint16_t address) = 0;
	virtual void chrWrite(uint16_t address, uint8_t value) = 0;
};

