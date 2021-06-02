#pragma once

#include <vector>

#include "CPU.h"
#include "PPU.h"
#include "Memory.h"
#include "../graphics/Graphics.h"
#include "../graphics/IDrawable.h"

class NES
{
public:
	// Initialize
	NES();

	// Attempt to initialize the window, and return whether success or failure
	bool init();

	// Main window event loop
	void run();

	// Close window on next loop
	void terminate();

private:
	int windowWidth, windowHeight;
	bool shouldTerminate;
	GLFWwindow *window;
	std::vector<IDrawable*> drawables;

	// NES Components
	Memory memory;
	CPU cpu;
	PPU ppu;
};