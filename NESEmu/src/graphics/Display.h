#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Display
{
public:
	Display();

	// Attempt to initialize the window, and return whether success or failure
	bool init();
	void loop();
	void terminate();

private:
	bool shouldTerminate;
	GLFWwindow *window;
};