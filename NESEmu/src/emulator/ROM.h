#pragma once

#include "Mapper.h"
#include "Memory.h"
#include "PPU.h"
#include "CPU.h"

#include <string>

// Why is this needed...?
class CPU;

// Representation of a ROM, which handles loading and dealing with files in the iNES format
class ROM
{
public:
	// iNES header (16 bytes)
	struct Header
	{
		char name[4];
		uint8_t prgBanks;
		uint8_t chrBanks;
		uint8_t mapperFlags1;
		uint8_t mapperFlags2;
		uint8_t prgRamSize;
		uint8_t tvFlags1;
		uint8_t tvFlags2;
		uint8_t _padding[5];
	} header;

	// PPU mirroring mode
	enum class Mirroring : uint8_t
	{
		Horizontal = 0,
		Vertical
	} mirroring;

	// Initialize ROM without any file
	ROM();

	// Initialize ROM with given file path
	ROM(std::string path);

	// Load ROM from given file path
	void load(std::string path);

	// Reads ROM from file path and maps it into memory based on mapper configuration
	void map(Bus &bus, CPU &cpu, PPU &ppu);

	// Returns the mapper ID associated with the ROM
	uint8_t getMapperID() const;

	// Returns the path of the ROM file on the disk
	std::string getPath() const;

private:
	// ID of the mapper needed for the ROM
	uint8_t mapperID;

	// ROM path on disk
	std::string path;

	// Mapper used for this ROM
	Mapper *mapper;
};

