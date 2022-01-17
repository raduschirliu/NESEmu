#pragma once

#include "../Window.h"
#include "../../emulator/PPU.h"

#include <sstream>

class PPUDebugWindow : public Window
{
public:
	PPUDebugWindow(PPU &ppu);

	void draw() override;

private:
	void printMemory(uint16_t start, uint16_t end);
	void drawPalette(std::string label, std::vector<PPU::Color> palette);
	void drawNametable(uint16_t start);

	PPU &ppu;
	std::stringstream ss;
};