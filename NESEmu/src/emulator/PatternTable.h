#pragma once

#include <array>
#include <cstdint>
#include <vector>

// Forward declaration
class PPU;

class PatternTable
{
public:
	// Pattern table is 16 x 16 tiles
	static constexpr uint16_t SIZE = 16;
	
	// Pattern table has 16 x 16 entries
	static constexpr uint16_t ENTRIES = 16 * 16;

	// A tile is 8 x 8 pixels
	static constexpr uint16_t TILE_SIZE = 8;

	// Represents a tile pattern in the pattern table
	using Pattern = std::array<uint8_t, TILE_SIZE * TILE_SIZE>;

	PatternTable();

	void load(PPU &ppu, uint16_t baseAddress);
	const Pattern &getPattern(size_t index) const;
	const Pattern &getPattern(size_t row, size_t col) const;

private:
	std::vector<Pattern> patterns;
};
