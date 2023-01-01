#pragma once

#include <sstream>

#include "emulator/Cartridge.h"
#include "graphics/Window.h"

class CartridgeDebugWindow : public Window
{
  public:
    CartridgeDebugWindow(Cartridge &cartridge);

    void draw() override;

  private:
    Cartridge &cartridge;
    std::stringstream ss;
};
