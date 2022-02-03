#include "Input.h"

using std::string;
using std::bitset;
using std::unordered_map;

unordered_map<int, bool> Input::keys;
unordered_map<string, Input::KeyMap> Input::keyMaps;

// GLFW key callback
static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

void Input::init(GLFWwindow *window)
{
	glfwSetKeyCallback(window, keyCallback);
}

void Input::registerKeyMap(string name, KeyMap keyMap)
{
	keyMaps[name] = keyMap;
}

bitset<Input::KEYMAP_SIZE> Input::getKeyMap(string name)
{
	bitset<KEYMAP_SIZE> bits(0);

	for (auto &pair : keyMaps[name])
	{
		if (keys[pair.first])
		{
			bits[static_cast<uint8_t>(pair.second)] = true;
		}
	}

	return bits;
}

void Input::onKeyEvent(int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = false;
	}
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	Input::onKeyEvent(key, scancode, action, mods);
}