#pragma once

#include <string>
#include <vector>

#include "Bus.h"
#include "PPU.h"

// Forward declarations
class IMapper;

// Representation of a game cartridge, which handles loading and dealing with
// files in the iNES format
class Cartridge
{
  public:
    // PRG ROM bank size: 16 KiB ($4000)
    static constexpr uint16_t PRG_BANK_SIZE = 0x4000;

    // CHR ROM bank size: 8 KiB ($2000)
    static constexpr uint16_t CHR_BANK_SIZE = 0x2000;

    // Constant at the start of every header: "NES" followed by MS-DOS EOF
    static constexpr uint8_t HEADER_NAME[] = {0x4E, 0x45, 0x53, 0x1A};

    // 512 byte trainer
    static constexpr uint16_t TRAINER_SIZE = 512;

    // iNES header (16 bytes)
    struct Header
    {
        char name[4];      // Must be == HEADER_NAME
        uint8_t prg_banks;  // PRG ROM banks in 16 KiB units
        uint8_t chr_banks;  // CHR ROM banks in 8 KiB units

        struct Flags6
        {
            uint8_t mirroring : 1;
            uint8_t has_prg_ram : 1;
            uint8_t has_trainer : 1;
            uint8_t ignore_mirroring : 1;
            uint8_t mapper_lower_nibble : 4;
        } flags6;  // Top - LSB, Bottom - MSB

        struct Flags7
        {
            uint8_t vs_unisystem : 1;
            uint8_t has_playchoice_data : 1;
            uint8_t format : 2;
            uint8_t mapper_upper_nibble : 4;
        } flags7;  // Top - LSB, Bottom - MSB

        uint8_t prg_ram_size;  // Size of PRG RAM in 8 KiB units
        uint8_t tv_flags_1;
        uint8_t tv_flags_2;

        uint8_t _padding[5];
    };

    // Initialize cartridge
    Cartridge();

    // Load ROM from given file path
    bool load(std::string path);

    // iNES ROM data getters
    Header GetHeader() const;
    std::vector<uint8_t> &GetPrgRom();
    std::vector<uint8_t> &GetChrRom();

    // Returns the path of the ROM file on the disk
    std::string GetPath() const;

    // Returns the mapper ID associated with the ROM
    uint8_t GetMapperId() const;

    // Returns the active mapper
    IMapper *GetMapper();

  private:
    // iNES ROM data
    Header header_;
    std::vector<uint8_t> prg_rom_;
    std::vector<uint8_t> chr_rom_;

    // ROM path on disk
    std::string path_;

    // Mapper used for this ROM
    IMapper *mapper_;
};
