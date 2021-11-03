#pragma once

#include "../Window.h"
#include "../../emulator/Memory.h"

#include <sstream>

class MemoryViewWindow : public Window
{
public:
	MemoryViewWindow(Memory &memory);

	void draw() override;

private:
	// Dump entire memory region to log file
	void printMemory(uint16_t start, uint16_t end);

	const int pageSize = 0x00FF;

	int currentPage;
	Memory &memory;
	std::stringstream ss;
};