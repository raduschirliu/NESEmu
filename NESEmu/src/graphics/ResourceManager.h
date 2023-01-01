#pragma once

#include <string>
#include <unordered_map>

#include "Shader.h"
#include "Texture.h"

class ResourceManager
{
  public:
    static bool loadShader(std::string name, std::string fragmentPath,
                           std::string vertexPath);
    static bool loadTexture(std::string name, std::string shaderName, int width,
                            int height);

    static Shader *getShader(std::string name);
    static Texture *getTexture(std::string name);

  private:
    static std::unordered_map<std::string, Shader *> shaders;
    static std::unordered_map<std::string, Texture *> textures;
};