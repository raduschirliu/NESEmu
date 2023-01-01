#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

#include "Graphics.h"
#include "Shader.h"
#include "emulator/PPU.h"

class Texture
{
  public:
    Texture(Shader *shader, int width, int height);
    Texture(const Texture &other);
    ~Texture();

    void load(PPU &ppu, uint16_t baseAddress);
    void update(PPU &ppu, uint16_t baseAddress);
    void draw(glm::vec2 pos, glm::vec2 size);
    // void draw(glm::vec2 pos, glm::vec2 size, glm::vec2 uvTopLeft, glm::vec2
    // uvBottomRight, std::vector<PPU::Color> palette);
    void draw(glm::vec3 pos, glm::vec2 size, glm::vec2 uvTopLeft,
              glm::vec2 uvBottomRight, std::vector<PPU::Color> palette,
              glm::vec4 color = glm::vec4(1.0f));

    void drawGui(ImVec2 size);
    void drawGui(ImVec2 size, ImVec2 texPosTopLeft, ImVec2 texPosBottomRight);

    GLuint getId();
    int getWidth();
    int getHeight();

  private:
    Shader *shader;
    GLuint textureId;
    GLuint vaoId, vboId, eboId;
    int width, height;

    std::vector<uint8_t> getPixelData(PPU &ppu, uint16_t baseAddress);
};