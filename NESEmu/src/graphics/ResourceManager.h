#pragma once

#include <string>
#include <unordered_map>

#include "Shader.h"
#include "Texture.h"

class ResourceManager
{
  public:
    static bool LoadShader(std::string name, std::string fragmentPath,
                           std::string vertexPath);
    static bool LoadTexture(std::string name, std::string shaderName, int width,
                            int height);

    static Shader *GetShader(std::string name);
    static Texture *GetTexture(std::string name);

  private:
    static std::unordered_map<std::string, Shader *> shaders_;
    static std::unordered_map<std::string, Texture *> textures_;
};