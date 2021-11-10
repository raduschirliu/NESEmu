#pragma once

#include "../Window.h"
#include "../../emulator/PPU.h"

class PPUDebugWindow : public Window
{
public:
	PPUDebugWindow(PPU &ppu);

	void draw() override;

private:
	PPU &ppu;
};