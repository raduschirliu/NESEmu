#pragma once

#include "../Window.h"
#include "../../emulator/Memory.h"

class MemoryViewWindow : public Window
{
public:
	MemoryViewWindow(Memory &memory);

	void draw() override;

private:
	Memory &memory;
};