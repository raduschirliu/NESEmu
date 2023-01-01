#include "Input.h"

using std::bitset;
using std::string;
using std::unordered_map;

unordered_map<int, bool> Input::keys_;
unordered_map<string, Input::KeyMap> Input::key_maps_;

// GLFW key callback
static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                        int mods);

void Input::Init(GLFWwindow *window)
{
    glfwSetKeyCallback(window, keyCallback);
}

void Input::RegisterKeyMap(string name, KeyMap keyMap)
{
    key_maps_[name] = keyMap;
}

bitset<Input::kKeymapSize> Input::GetKeyMap(string name)
{
    bitset<kKeymapSize> bits(0);

    for (auto &pair : key_maps_[name])
    {
        if (keys_[pair.first])
        {
            bits[static_cast<uint8_t>(pair.second)] = true;
        }
    }

    return bits;
}

void Input::OnKeyEvent(int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        keys_[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        keys_[key] = false;
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods)
{
    Input::OnKeyEvent(key, scancode, action, mods);
}