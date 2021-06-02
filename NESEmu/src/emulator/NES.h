#pragma once

#include <vector>

#include "CPU.h"
#include "PPU.h"
#include "Memory.h"
#include "ROM.h"
#include "../graphics/Graphics.h"
#include "../graphics/IDrawable.h"

class NES
{
public:
	// Initialize
	NES();

	// Attempt to initialize the window, and return whether success or failure
	bool init();

	// Load a ROM into memory from given path
	void load(std::string path);

	// Main window event loop
	void run();

	// Close window on next loop
	void shutdown();

private:
	int windowWidth, windowHeight;
	bool shouldShutdown;
	GLFWwindow *window;
	std::vector<IDrawable*> drawables;

	// NES Components
	Memory memory;
	ROM rom;
	CPU cpu;
	PPU ppu;
};