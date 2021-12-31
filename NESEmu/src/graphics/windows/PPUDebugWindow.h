#pragma once

#include "../Window.h"
#include "../../emulator/PPU.h"

class PPUDebugWindow : public Window
{
public:
	PPUDebugWindow(PPU &ppu);

	void draw() override;

private:
	void printMemory(std::stringstream& ss, uint16_t start, uint16_t end);

	PPU &ppu;
};