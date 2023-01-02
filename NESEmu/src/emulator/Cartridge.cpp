#include "Cartridge.h"

#include <cerrno>
#include <fstream>

#include "MapperFactory.h"

Cartridge::Cartridge() : header_({0}), path_(""), mapper_(nullptr)
{
}

bool Cartridge::load(std::string path)
{
    this->path_ = path;
    std::ifstream stream;
    stream.open(path, std::ifstream::binary);

    if (!stream.is_open())
    {
        printf("Failed to open ROM file: %s\n", path.c_str());
        return false;
    }

    // Read header
    stream.read((char *)&header_, sizeof(Header));

    if (memcmp(header_.name, HEADER_NAME, sizeof(HEADER_NAME)) != 0)
    {
        printf("Error, cartridge loaded ROM with invalid header constant\n");
        return false;
    }

    // Check if trainer is present
    if (header_.flags6.has_trainer)
    {
        stream.seekg(TRAINER_SIZE, std::ios_base::cur);
    }

    // TODO: Make more efficient

    // Read PRG ROM in 16 KiB ($4000) increments
    uint16_t prgRomSize = 0x4000 * header_.prg_banks;
    for (uint32_t i = 0; i < prgRomSize; i++)
    {
        uint8_t byte;
        stream.read((char *)&byte, 1);
        prg_rom_.push_back(byte);
    }

    // Read CHR ROM in 8 KiB ($2000) increments
    uint16_t chrRomSize = 0x2000 * header_.chr_banks;
    for (uint32_t i = 0; i < chrRomSize; i++)
    {
        uint8_t byte;
        stream.read((char *)&byte, 1);
        chr_rom_.push_back(byte);
    }

    // Create mapper
    if (mapper_)
    {
        delete mapper_;
    }

    mapper_ = MapperFactory::CreateMapper(*this);

    if (!mapper_)
    {
        printf("Error, mapper %u not implemented yet!\n", GetMapperId());
        return false;
    }

    printf("Loaded ROM: %s\n", path.c_str());
    printf("\tMapper: %u\n", GetMapperId());
    printf("\tMirroring: %u\n", mapper_->GetMirroringMode());
    printf("\tPRG ROM size: %u banks -> %u bytes\n", header_.prg_banks,
           prg_rom_.size());
    printf("\tCHR ROM size: %u banks -> %u bytes\n", header_.chr_banks,
           chr_rom_.size());
    return true;
}

Cartridge::Header Cartridge::GetHeader() const
{
    return header_;
}

std::vector<uint8_t> &Cartridge::GetPrgRom()
{
    return prg_rom_;
}

std::vector<uint8_t> &Cartridge::GetChrRom()
{
    return chr_rom_;
}

std::string Cartridge::GetPath() const
{
    return path_;
}

uint8_t Cartridge::GetMapperId() const
{
    return header_.flags6.mapper_lower_nibble &
           (header_.flags7.mapper_upper_nibble << 4);
}

IMapper *Cartridge::GetMapper()
{
    return mapper_;
}