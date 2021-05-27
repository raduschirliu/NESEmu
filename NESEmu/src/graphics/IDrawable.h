#pragma once

class IDrawable
{
public:
	// Perform drawing for the component every frame
	virtual void draw() = 0;

	bool getEnabled() const { return enabled; }

	void enable() { enabled = true; }
	void disable() { enabled = false; }

private:
	// Whether the window is enabled or not
	bool enabled = false;
};