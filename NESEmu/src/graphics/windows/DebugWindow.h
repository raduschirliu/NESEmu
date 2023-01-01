#pragma once

#include "emulator/CPU.h"
#include "emulator/NES.h"
#include "graphics/Window.h"

class DebugWindow : public Window
{
  public:
    DebugWindow(NES &nes, CPU &cpu);
    void Draw() override;

  private:
    double prevTime, emulationSpeed;
    int renderingScale;
    uint32_t frames, fps;

    NES &nes;
    CPU &cpu;
};