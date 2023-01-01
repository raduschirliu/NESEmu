#pragma once

#include "Graphics.h"
#include "IDrawable.h"

class Window : public IDrawable
{
  public:
    // Allow windows to be toggled by keypress
    virtual void Update() override
    {
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(key_)))
        {
            visible_ = !visible_;
        }
    }

    // Control whether the window is enabled /visible
    bool IsVisible() override
    {
        return visible_;
    }

    bool IsActive() override
    {
        return true;
    }

    void SetVisible(bool state)
    {
        visible_ = state;
    }

  protected:
    Window(int key) : key_(key)
    {
    }

    // Key to be pressed to toggle window
    int key_ = -1;

    // Whether the window is visible or not
    bool visible_ = false;
};