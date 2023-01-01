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

    void Load(PPU &ppu, uint16_t base_address);
    void Update(PPU &ppu, uint16_t base_address);
    void Draw(glm::vec2 pos, glm::vec2 size);
    // void draw(glm::vec2 pos, glm::vec2 size, glm::vec2 uvTopLeft, glm::vec2
    // uv_bottom_right, std::vector<PPU::Color> palette);
    void Draw(glm::vec3 pos, glm::vec2 size, glm::vec2 uv_top_left,
              glm::vec2 uv_bottom_right, std::vector<PPU::Color> palette,
              glm::vec4 color = glm::vec4(1.0f));

    void DrawGui(ImVec2 size);
    void DrawGui(ImVec2 size, ImVec2 tex_pos_top_left,
                 ImVec2 tex_pos_bottom_right);

    GLuint GetId();
    int GetWidth();
    int GetHeight();

  private:
    Shader *shader;
    GLuint textureId;
    GLuint vaoId, vboId, eboId;
    int width, height;

    std::vector<uint8_t> getPixelData(PPU &ppu, uint16_t base_address);
};