#pragma once

#include <sstream>

#include "emulator/NES.h"
#include "emulator/PPU.h"
#include "graphics/Texture.h"
#include "graphics/Window.h"

class PPUDebugWindow : public Window
{
  public:
    PPUDebugWindow(NES &nes, PPU &ppu, Cartridge &cartridge);

    void draw() override;

  private:
    void drawRegister(std::string name, uint16_t address, const void *reg,
                      const char *helpText);
    void drawPalette(std::string label, std::vector<PPU::Color> palette);
    void drawNametable(uint8_t nametable);
    void drawOam();

    NES &nes;
    PPU &ppu;
    Cartridge &cartridge;
    Texture *patternTableLeft;
    Texture *patternTableRight;
    std::stringstream ss;
    int debugViewNametable;
};