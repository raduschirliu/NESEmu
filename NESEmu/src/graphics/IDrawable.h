#pragma once

class IDrawable
{
  public:
    // Update prior to drawing
    virtual void Update()
    {
    }

    // Perform drawing for the component every frame
    virtual void Draw() = 0;

    // Whether the component should be updated at all
    virtual bool IsActive() = 0;

    // Whether the component should be drawn (only valid if IsActive() == true)
    virtual bool IsVisible() = 0;
};