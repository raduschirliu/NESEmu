#pragma once

#include "CPU.h"
#include "Memory.h"
#include "PPU.h"
#include "../graphics/Graphics.h"
#include "../graphics/IDrawable.h"

#include <vector>

class NES
{
public:
	NES();

	// Load game ROM
	void load(std::string path);

	// Attempt to initialize window, return whether success or failure
	bool init();

	// Start the NES, open the window, start draw loop
	void run();

	// Stop the NES
	void shutdown();

private:
	int windowWidth, windowHeight;
	bool shouldShutdown;
	std::vector<IDrawable> drawables;
	GLFWwindow *window;
	Memory memory;
	CPU cpu;
	PPU ppu;
	ROM rom;
};