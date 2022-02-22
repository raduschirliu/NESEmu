#include "PatternTable.h"
#include "PPU.h"

#include <assert.h>

PatternTable::PatternTable() : patterns(ENTRIES, { 0 })
{
	
}

void PatternTable::load(PPU &ppu, uint16_t baseAddress)
{
	for (size_t patternIndex = 0; patternIndex < ENTRIES; patternIndex++)
	{
		Pattern pattern = { 0 };

		for (size_t byteIndex = 0; byteIndex < 8; byteIndex++)
		{
			for (size_t bitIndex = 0; bitIndex < 8; bitIndex++)
			{
				uint16_t address = baseAddress | (patternIndex << 4) | byteIndex;
				uint8_t loByte = ppu.readMemory(address);
				uint8_t hiByte = ppu.readMemory(address + 8);
				uint8_t mask = 1 << bitIndex;

				// Account for the edge case where bitIndex == 0, and the high bit needs to be shifted
				// 1 to the left so that it doesn't occupy the same place as the low bit
				uint8_t hiBit = bitIndex == 0 ?
					((hiByte & mask) << 1) :
					((hiByte & mask) >> (bitIndex - 1));
				uint8_t bit = hiBit | ((loByte & mask) >> bitIndex);

				size_t pixelIndex = byteIndex * 8 + 7 - bitIndex;
				pattern[pixelIndex] = bit;
			}
		}

		patterns[patternIndex] = pattern;
	}
}

const PatternTable::Pattern &PatternTable::getPattern(size_t index) const
{
	assert(index < ENTRIES);
	return patterns[index];
}

const PatternTable::Pattern &PatternTable::getPattern(size_t row, size_t col) const
{
	assert(row < SIZE && col < SIZE);
	return getPattern(row * SIZE + col);
}
