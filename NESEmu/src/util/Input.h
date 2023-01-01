#pragma once

#include <bitset>
#include <string>
#include <unordered_map>

#include "emulator/Controller.h"
#include "graphics/Graphics.h"

class Input
{
  public:
    // Since the NES input registers are 8 bits, keymaps are also 8 bits
    static constexpr size_t kKeymapSize = 8;
    using KeyMap = std::unordered_map<int, Controller::Button>;

    // Start capturing input events
    static void Init(GLFWwindow *window);
    static void RegisterKeyMap(std::string name, KeyMap keyMap);
    static std::bitset<kKeymapSize> GetKeyMap(std::string name);
    static void OnKeyEvent(int key, int scancode, int action, int mods);

  private:
    static std::unordered_map<int, bool> keys_;
    static std::unordered_map<std::string, KeyMap> key_maps_;
};