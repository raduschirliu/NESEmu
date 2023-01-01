#include "ResourceManager.h"

using std::string;
using std::unordered_map;

unordered_map<string, Shader *> ResourceManager::shaders_;
unordered_map<string, Texture *> ResourceManager::textures_;

bool ResourceManager::LoadShader(string name, string fragmentPath,
                                 string vertexPath)
{
    if (GetShader(name))
    {
        printf("[Error] Failed to load shader '%s' - Already exists\n",
               name.c_str());
        return false;
    }

    Shader *shader = new Shader();
    shader->load(fragmentPath, vertexPath);
    shaders_.insert({name, shader});

    GL_ERROR_CHECK();

    printf("Loaded shader '%s' from '%s', '%s'\n", name.c_str(),
           fragmentPath.c_str(), vertexPath.c_str());
    return true;
}

bool ResourceManager::LoadTexture(string name, string shaderName, int width,
                                  int height)
{
    if (GetTexture(name))
    {
        printf("[Error] Failed to load texture '%s' - Already exists\n",
               name.c_str());
        return false;
    }

    if (!GetShader(shaderName))
    {
        printf(
            "[Error] Failed to load texture '%s' - shader '%s' doesn't exist\n",
            shaderName.c_str(), name.c_str());
        return false;
    }

    textures_.insert({name, new Texture(shaders_[shaderName], width, height)});
    printf("Loaded texture '%s' with shader '%s'\n", name.c_str(),
           shaderName.c_str());
    return true;
}

Shader *ResourceManager::GetShader(string name)
{
    auto it = shaders_.find(name);

    if (it == shaders_.end())
    {
        return nullptr;
    }

    return it->second;
}

Texture *ResourceManager::GetTexture(string name)
{
    auto it = textures_.find(name);

    if (it == textures_.end())
    {
        return nullptr;
    }

    return it->second;
}