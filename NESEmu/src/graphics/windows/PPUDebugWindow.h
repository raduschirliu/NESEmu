#pragma once

#include "../Window.h"
#include "../../emulator/NES.h"
#include "../../emulator/PPU.h"
#include "../Texture.h"

#include <sstream>

class PPUDebugWindow : public Window
{
public:
	PPUDebugWindow(NES &nes, PPU &ppu);

	void draw() override;

private:
	void printMemory(uint16_t start, uint16_t end);
	void drawPalette(std::string label, std::vector<PPU::Color> palette);
	void drawNametable(uint8_t nametable);

	NES &nes;
	PPU &ppu;
	Texture *patternTableLeft;
	Texture *patternTableRight;
	std::stringstream ss;
	int debugViewNametable;
};