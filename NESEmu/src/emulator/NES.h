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

	// Emulate one single NES cycle step
	void step();

	// Close window on next loop
	void shutdown();

	// Set the 'running' state of NES
	void setRunning(bool running);

	// Gets whether the NES is running or paused
	bool getRunning() const;

	// Put the NES into debug mode. Load nestest, and put into automated test mode
	void loadDebugMode();

	// Set emulation speed
	void setEmulationSpeed(double speed);

	// Returns a copy of the NES' ROM
	ROM& getRom();

private:
	// Window dimensions
	int windowWidth, windowHeight;
	
	// Whether the emulation is running or paused
	bool running;

	// Whether the emulator window should close or not
	bool shouldShutdown;

	// Emulation speed
	double emulationSpeed;

	// GLFW window handle
	GLFWwindow *window;

	// List of all drawable components
	std::vector<IDrawable*> drawables;

	// NES Components
	Memory memory;
	ROM rom;
	CPU cpu;
	PPU ppu;
};