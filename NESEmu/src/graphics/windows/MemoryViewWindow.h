#pragma once

#include <sstream>

#include "emulator/Bus.h"
#include "graphics/Window.h"

class MemoryViewWindow : public Window
{
  public:
    MemoryViewWindow(Bus &bus);

    void draw() override;

  private:
    const int pageSize = 0x00FF;

    int currentPage;
    Bus &bus;
    std::stringstream ss;
};