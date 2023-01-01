#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using std::vector;

static vector<float> NormalizePalette(vector<PPU::Color> palette);

vector<float> NormalizePalette(vector<PPU::Color> palette)
{
    vector<float> normalized;

    for (auto color : palette)
    {
        normalized.push_back(color.r / 255.0f);
        normalized.push_back(color.g / 255.0f);
        normalized.push_back(color.b / 255.0f);
        normalized.push_back(color.a / 255.0f);
    }

    return normalized;
}

Texture::Texture(Shader *shader, int width, int height)
    : shader_(shader),
      texture_id_(0),
      vao_id_(0),
      vbo_id_(0),
      ebo_id_(0),
      width(width),
      height(height)
{
}

Texture::Texture(const Texture &other)
    : shader_(other.shader_),
      texture_id_(other.texture_id_),
      vao_id_(other.vao_id_),
      vbo_id_(other.vbo_id_),
      ebo_id_(other.ebo_id_),
      width(other.width),
      height(other.height)
{
}

Texture::~Texture()
{
    if (texture_id_ != 0)
    {
        glDeleteBuffers(1, &ebo_id_);
        glDeleteBuffers(1, &vbo_id_);
        glDeleteVertexArrays(1, &vao_id_);
        glDeleteTextures(1, &texture_id_);
    }
}

void Texture::Load(PPU &ppu, uint16_t base_address)
{
    // Create and bind buffers
    glGenBuffers(1, &vbo_id_);
    glGenBuffers(1, &ebo_id_);
    glGenVertexArrays(1, &vao_id_);

    assert(vbo_id_ != 0);
    assert(ebo_id_ != 0);
    assert(vao_id_ != 0);
    GL_ERROR_CHECK();

    glBindVertexArray(vao_id_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id_);
    glBindVertexArray(0);

    GL_ERROR_CHECK();

    vector<uint8_t> pixels = getPixelData(ppu, base_address);

    // Create OpenGL texture identifier
    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);

    // Setup filtering parameters for display (using nearest for pixelated
    // textures)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, &pixels[0]);

    glBindTexture(GL_TEXTURE_2D, 0);

    GL_ERROR_CHECK();
}

void Texture::Update(PPU &ppu, uint16_t base_address)
{
    vector<uint8_t> pixels = getPixelData(ppu, base_address);

    glBindTexture(GL_TEXTURE_2D, texture_id_);
    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB,
                    GL_UNSIGNED_BYTE, &pixels[0]);
    glBindTexture(GL_TEXTURE_2D, 0);

    GL_ERROR_CHECK();
}

void Texture::Draw(glm::vec2 pos, glm::vec2 size)
{
    vector<PPU::Color> palette(4);
    // Draw(pos, size, glm::vec2(0, 0), glm::vec2(width, height), palette);
    Draw(glm::vec3(pos, 0.0f), size, glm::vec2(0.0f), glm::vec2(width, height),
         palette, glm::vec4(1.0f));
}

/*
void Texture::Draw(glm::vec2 pos, glm::vec2 size, glm::vec2 uvTopLeft, glm::vec2
uvBottomRight, std::vector<PPU::Color> palette)
{
        Draw(glm::vec3(pos.x, pos.y, 0.0f), size, uvTopLeft, uvBottomRight,
palette);
}
*/

void Texture::Draw(glm::vec3 pos, glm::vec2 size, glm::vec2 uvTopLeft,
                   glm::vec2 uvBottomRight, vector<PPU::Color> palette,
                   glm::vec4 color)
{
    assert(palette.size() == 4);

    // Enable shader and set uniforms
    shader_->use();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
    // TODO: Update projection matrix when window is changed
    glm::mat4 projection =
        glm::ortho<float>(0.0f, 1280.0f, 720.0f, 0.0f, -10.0f, 10.0f);

    // TODO: Cache normalized float palette
    vector<float> normalizedPalette = NormalizePalette(palette);
    assert(normalizedPalette.size() == palette.size() * 4);

    shader_->setVector4f("colorModifier", color);
    shader_->setVector4f("palette", normalizedPalette);
    shader_->setMatrix4f("model", model);
    shader_->setMatrix4f("projection", projection);

    // Enable texture alpha, depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glBindVertexArray(vao_id_);

    // Update VAO
    // For UVs: bottom left = (0, 0), top right = (1.0, 1.0)
    float vertices[] = {
        // Position				// Texture coords
        1.0f,
        1.0f,
        0.0f,
        uvBottomRight.x / width,
        uvBottomRight.y / height,  // top right
        1.0f,
        0.0f,
        0.0f,
        uvBottomRight.x / width,
        uvTopLeft.y / height,  // bottom right
        0.0f,
        0.0f,
        0.0f,
        uvTopLeft.x / width,
        uvTopLeft.y / height,  // bottom left
        0.0f,
        1.0f,
        0.0f,
        uvTopLeft.x / width,
        uvBottomRight.y / height,  // top left
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Update EBO
    unsigned int indices[] = {
        0, 1, 3,  // First triangle
        1, 2, 3   // Second triangle
    };

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    // Draw vertices
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // Unbind, disable, and cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    shader_->abandon();

    GL_ERROR_CHECK();
}

void Texture::DrawGui(ImVec2 size)
{
    DrawGui(size, ImVec2(0.0f, 0.0f), ImVec2(width, height));
}

void Texture::DrawGui(ImVec2 size, ImVec2 texPosTopLeft,
                      ImVec2 texPosBottomRight)
{
    // For ImGui: Top left = (0.0, 0.0), bottom right = (1.0, 1.0)
    ImVec2 uvStart(texPosTopLeft.x / width, texPosTopLeft.y / height);
    ImVec2 uvEnd(texPosBottomRight.x / width, texPosBottomRight.y / height);

    ImGui::Image((void *)(intptr_t)texture_id_, size, uvStart, uvEnd);
}

GLuint Texture::GetId()
{
    return texture_id_;
}

int Texture::GetWidth()
{
    return width;
}

int Texture::GetHeight()
{
    return height;
}

vector<uint8_t> Texture::getPixelData(PPU &ppu, uint16_t base_address)
{
    vector<uint8_t> pixels;
    pixels.reserve(128 * 128 * 3);

    uint8_t colors[] = {0, 51, 102, 153};  // { 0.0, 0.2, 0.4, 0.6 } * 255

    for (uint8_t r = 0; r < 128; r++)
    {
        for (uint8_t c = 0; c < 128; c++)
        {
            uint16_t patternIndex =
                16 * (r / 8) + c / 8;  // Which pattern index to use
            uint16_t byteIndex =
                r % 8;  // Which byte of the pattern (row of pixels)
            uint8_t bitIndex =
                7 - c % 8;  // The bit (pixel) to use in the current byte

            uint16_t address = base_address | (patternIndex << 4) | byteIndex;
            uint8_t loByte = ppu.readMemory(address);
            uint8_t hiByte = ppu.readMemory(address + 8);
            uint8_t mask = 1 << bitIndex;

            // Account for the edge case where bitIndex == 0, and the high bit
            // needs to be shifted 1 to the left so that it doesn't occupy the
            // same place as the low bit
            uint8_t hiBit = bitIndex == 0 ? ((hiByte & mask) << 1)
                                          : ((hiByte & mask) >> (bitIndex - 1));
            uint8_t bit = hiBit | ((loByte & mask) >> bitIndex);

            pixels.push_back(colors[bit]);
            pixels.push_back(colors[bit]);
            pixels.push_back(colors[bit]);
        }
    }

    return pixels;
}