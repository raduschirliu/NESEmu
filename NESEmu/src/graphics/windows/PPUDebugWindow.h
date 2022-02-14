#pragma once

#include "../Window.h"
#include "../../emulator/NES.h"
#include "../../emulator/PPU.h"
#include "../Texture.h"

#include <sstream>

class PPUDebugWindow : public Window
{
public:
	PPUDebugWindow(NES &nes, PPU &ppu, Cartridge &cartridge);

	void draw() override;

private:
	void drawRegister(std::string name, uint16_t address, const void* reg, const char* helpText);
	void drawPalette(std::string label, const Palette& palette);
	void drawNametable(uint8_t nametable);
	void drawOam();

	NES &nes;
	PPU &ppu;
	Cartridge &cartridge;
	Texture *patternTableLeft;
	Texture *patternTableRight;
	std::stringstream ss;
	int debugViewNametable;
};