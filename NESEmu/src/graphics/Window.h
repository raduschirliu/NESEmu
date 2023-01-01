#pragma once

#include "Graphics.h"
#include "IDrawable.h"

class Window : public IDrawable
{
  public:
    // Allow windows to be toggled by keypress
    virtual void update() override
    {
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(key)))
        {
            visible = !visible;
        }
    }

    // Control whether the window is enabled /visible
    bool isVisible() override
    {
        return visible;
    }

    bool isActive() override
    {
        return true;
    }

    void setVisible(bool state)
    {
        visible = state;
    }

  protected:
    Window(int key) : key(key)
    {
    }

    // Key to be pressed to toggle window
    int key = -1;

    // Whether the window is visible or not
    bool visible = false;
};