#pragma once

#include "IDrawable.h"
#include "../emulator/CPU.h"

class CpuWindow : public IDrawable
{
public:
	CpuWindow(CPU &cpu);
	void draw() override;

private:
	CPU &cpu;
};