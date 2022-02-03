#pragma once

#include "../graphics/Graphics.h"
#include "../emulator/Controller.h"

#include <bitset>
#include <unordered_map>
#include <string>

class Input
{
public:
	// Since the NES input registers are 8 bits, keymaps are also 8 bits
	static constexpr size_t KEYMAP_SIZE = 8;
	using KeyMap = std::unordered_map<int, Controller::Button>;

	// Start capturing input events
	static void init(GLFWwindow *window);
	static void registerKeyMap(std::string name, KeyMap keyMap);
	static std::bitset<KEYMAP_SIZE> getKeyMap(std::string name);
	static void onKeyEvent(int key, int scancode, int action, int mods);

private:
	static std::unordered_map<int, bool> keys;
	static std::unordered_map<std::string, KeyMap> keyMaps;
};