#pragma once

#include "emulator/Controller.h"
#include "graphics/Window.h"
#include "util/Input.h"

class InputDebugWindow : public Window
{
  public:
    InputDebugWindow(Controller &controller);

    void Draw() override;

  private:
    Controller &controller;
};