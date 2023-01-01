#include "Cartridge.h"

#include <cerrno>
#include <fstream>

#include "MapperFactory.h"

Cartridge::Cartridge() : header({0}), path(""), mapper(nullptr)
{
}

bool Cartridge::load(std::string path)
{
    this->path = path;
    std::ifstream stream;
    stream.open(path, std::ifstream::binary);

    if (!stream.is_open())
    {
        printf("Failed to open file\n");
        return false;
    }

    // Read header
    stream.read((char *)&header, sizeof(Header));

    if (memcmp(header.name, HEADER_NAME, sizeof(HEADER_NAME)) != 0)
    {
        printf("Error, cartridge loaded ROM with invalid header constant\n");
        return false;
    }

    // Check if trainer is present
    if (header.flags6.hasTrainer)
    {
        stream.seekg(TRAINER_SIZE, std::ios_base::cur);
    }

    // TODO: Make more efficient

    // Read PRG ROM in 16 KiB ($4000) increments
    uint16_t prgRomSize = 0x4000 * header.prgBanks;
    for (uint32_t i = 0; i < prgRomSize; i++)
    {
        uint8_t byte;
        stream.read((char *)&byte, 1);
        prgRom.push_back(byte);
    }

    // Read CHR ROM in 8 KiB ($2000) increments
    uint16_t chrRomSize = 0x2000 * header.chrBanks;
    for (uint32_t i = 0; i < chrRomSize; i++)
    {
        uint8_t byte;
        stream.read((char *)&byte, 1);
        chrRom.push_back(byte);
    }

    // Create mapper
    if (mapper)
    {
        delete mapper;
    }

    mapper = MapperFactory::CreateMapper(*this);

    if (!mapper)
    {
        printf("Error, mapper %u not implemented yet!\n", getMapperID());
        return false;
    }

    printf("Loaded ROM: %s\n", path.c_str());
    printf("\tMapper: %u\n", getMapperID());
    printf("\tMirroring: %u\n", mapper->GetMirroringMode());
    printf("\tPRG ROM size: %u banks -> %u bytes\n", header.prgBanks,
           prgRom.size());
    printf("\tCHR ROM size: %u banks -> %u bytes\n", header.chrBanks,
           chrRom.size());
    return true;
}

Cartridge::Header Cartridge::getHeader() const
{
    return header;
}

std::vector<uint8_t> &Cartridge::getPrgRom()
{
    return prgRom;
}

std::vector<uint8_t> &Cartridge::getChrRom()
{
    return chrRom;
}

std::string Cartridge::getPath() const
{
    return path;
}

uint8_t Cartridge::getMapperID() const
{
    return header.flags6.mapperLowerNibble &
           (header.flags7.mapperUpperNibble << 4);
}

IMapper *Cartridge::getMapper()
{
    return mapper;
}