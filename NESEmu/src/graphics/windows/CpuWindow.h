#pragma once

#include "../Window.h"
#include "../../emulator/CPU.h"

class CpuWindow : public Window
{
public:
	CpuWindow(CPU &cpu);
	void draw() override;

private:
	CPU &cpu;
};