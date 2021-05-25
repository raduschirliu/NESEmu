#pragma once

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class Display
{
public:
	// Initialize
	Display();

	// Attempt to initialize the window, and return whether success or failure
	bool init();

	// Main window event loop
	void loop();

	// Close window on next loop
	void terminate();

private:
	int windowWidth, windowHeight;
	bool shouldTerminate;
	GLFWwindow *window;
};