#pragma once

#include "../Window.h"
#include "../../emulator/NES.h"
#include "../../emulator/CPU.h"

class DebugWindow : public Window
{
public:
	DebugWindow(NES &nes, CPU &cpu);
	void draw() override;

private:
	double prevTime, emulationSpeed;
	int renderingScale;
	uint32_t frames, fps;

	NES &nes;
	CPU &cpu;
};