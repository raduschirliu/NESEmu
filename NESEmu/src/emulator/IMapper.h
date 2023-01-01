#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "MirroringMode.h"

// Forward declaration
class Cartridge;

// Base interface for mapper representation
class IMapper
{
  public:
    IMapper(Cartridge &cartridge)
    {
    }

    virtual uint8_t GetId() = 0;
    virtual std::string GetName() = 0;
    virtual MirroringMode GetMirroringMode() = 0;

    // Return true if the cartridge will handle the read/write operation
    virtual bool NametableRead(uint16_t address, uint8_t &value) = 0;
    virtual bool NametableWrite(uint16_t address, uint8_t value) = 0;

    // PRG memory operations
    virtual uint8_t PrgRead(uint16_t address) = 0;
    virtual void PrgWrite(uint16_t address, uint8_t value) = 0;

    // CHR memory operations
    virtual uint8_t ChrRead(uint16_t address) = 0;
    virtual void ChrWrite(uint16_t address, uint8_t value) = 0;
};
