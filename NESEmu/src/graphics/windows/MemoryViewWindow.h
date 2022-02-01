#pragma once

#include "../Window.h"
#include "../../emulator/Bus.h"

#include <sstream>

class MemoryViewWindow : public Window
{
public:
	MemoryViewWindow(Bus &bus);

	void draw() override;

private:
	// Dump entire memory region to log file
	void printMemory(uint16_t start, uint16_t end);

	const int pageSize = 0x00FF;

	int currentPage;
	Bus &bus;
	std::stringstream ss;
};