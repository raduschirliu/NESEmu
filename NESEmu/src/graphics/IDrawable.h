#pragma once

class IDrawable
{
public:
	// Update prior to drawing
	virtual void update() {}

	// Perform drawing for the component every frame
	virtual void draw() = 0;

	// Whether the component should be updated at all
	virtual bool isActive() = 0;

	// Whether the component should be drawn (only valid if isActive() == true)
	virtual bool isVisible() = 0;
};