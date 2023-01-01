#pragma once

#include <vector>

#include "emulator/IMapper.h"

namespace mappers
{
  
// Mapper 0: NROM
class NROM : public IMapper
{
  public:
    NROM(Cartridge &cartridge);

    uint8_t GetId() override;
    std::string GetName() override;
    MirroringMode GetMirroringMode() override;

    // Return true if the cartridge will handle the read/write operation
    bool NametableRead(uint16_t address, uint8_t &value) override;
    bool NametableWrite(uint16_t address, uint8_t value) override;

    // PRG memory operations
    uint8_t PrgRead(uint16_t address) override;
    void PrgWrite(uint16_t address, uint8_t value) override;

    // CHR memory operations
    uint8_t ChrRead(uint16_t address) override;
    void ChrWrite(uint16_t address, uint8_t value) override;

  private:
    Cartridge &cartridge_;
    std::vector<uint8_t> prg_ram_;
};

}  // namespace mappers
