#pragma once

class IDrawable
{
public:
	// Update prior to drawing
	virtual void update() {}

	// Perform drawing for the component every frame
	virtual void draw() = 0;
};