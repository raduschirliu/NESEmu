#pragma once

#include "Mapper.h"
#include "Bus.h"
#include "PPU.h"
#include "CPU.h"

#include <string>

// Forward declaration
class CPU;

// Representation of a game cartridge, which handles loading and dealing with files in the iNES format
class Cartridge
{
public:
	// Constant at the start of every header: "NES" followed by MS-DOS EOF
	static constexpr uint8_t HEADER_NAME[] = { 0x4E, 0x45, 0x53, 0x1A };

	// 512 byte trainer
	static constexpr uint16_t TRAINER_SIZE = 512;

	// iNES header (16 bytes)
	struct Header
	{
		char name[4]; // Must be == HEADER_NAME
		uint8_t prgBanks; // PRG ROM banks in 16 KB units
		uint8_t chrBanks; // CHR ROM banks in 8 KB units
		struct Flags6 {
			uint8_t mirroring : 1;
			uint8_t hasPrgRam : 1;
			uint8_t hasTrainer : 1;
			uint8_t ignoreMirroring : 1;
			uint8_t mapperLowerNibble : 4;
		} flags6; // Top - LSB, Bottom - MSB
		struct Flags7 {
			uint8_t vsUnisystem : 1;
			uint8_t hasPlaychoiceData : 1;
			uint8_t usesNes2Format : 2;
			uint8_t mapperUpperNibble : 4;
		} flags7; // Top - LSB, Bottom - MSB
		uint8_t prgRamSize; // Size of PRG RAM in 8 KB units
		uint8_t tvFlags1;
		uint8_t tvFlags2;
		uint8_t _padding[5];
	};

	// Initialize cartridge
	Cartridge();

	// Load ROM from given file path
	void load(std::string path);

	// Reads ROM from file path and maps it into memory based on mapper configuration
	void map(Bus &bus, CPU &cpu, PPU &ppu);

	// Returns the ROM headers
	Header getHeader() const;

	// Returns the mapper ID associated with the ROM
	uint8_t getMapperID() const;

	// Returns the path of the ROM file on the disk
	std::string getPath() const;

private:
	// iNES headers
	Header header;

	// ROM path on disk
	std::string path;

	// Mapper used for this ROM
	Mapper *mapper;
};

