#pragma once

#include "graphics/Window.h"

class DemoWindow : public Window
{
  public:
    // Make window openable using Insert
    DemoWindow() : Window(GLFW_KEY_INSERT)
    {
    }

    // Show ImGui demo window
    void Draw() override
    {
        ImGui::ShowDemoWindow(&visible_);
    }
};