#pragma once

#include "IDrawable.h"
#include "Graphics.h"

class Window : public IDrawable
{
public:
	Window(int key) : key(key) { }

	// Allow windows to be toggled by keypress
	virtual void update() override
	{
		if (key >= 0 && ImGui::IsKeyPressed(key))
		{
			enabled = !enabled;
		}
	}

	// Control whether the window is enabled and will display or not
	bool getEnabled() const { return enabled; }
	void enable() { enabled = true; }
	void disable() { enabled = false; }

protected:
	// Key to be pressed to toggle window
	int key = -1;

	// Whether the window is enabled or not
	bool enabled = false;
};