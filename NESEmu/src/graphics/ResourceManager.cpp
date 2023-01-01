#include "ResourceManager.h"

using std::string;
using std::unordered_map;

unordered_map<string, Shader *> ResourceManager::shaders;
unordered_map<string, Texture *> ResourceManager::textures;

bool ResourceManager::loadShader(string name, string fragmentPath,
                                 string vertexPath)
{
    if (getShader(name))
    {
        printf("[Error] Failed to load shader '%s' - Already exists\n",
               name.c_str());
        return false;
    }

    Shader *shader = new Shader();
    shader->load(fragmentPath, vertexPath);
    shaders.insert({name, shader});

    GL_ERROR_CHECK();

    printf("Loaded shader '%s' from '%s', '%s'\n", name.c_str(),
           fragmentPath.c_str(), vertexPath.c_str());
    return true;
}

bool ResourceManager::loadTexture(string name, string shaderName, int width,
                                  int height)
{
    if (getTexture(name))
    {
        printf("[Error] Failed to load texture '%s' - Already exists\n",
               name.c_str());
        return false;
    }

    if (!getShader(shaderName))
    {
        printf(
            "[Error] Failed to load texture '%s' - shader '%s' doesn't exist\n",
            shaderName.c_str(), name.c_str());
        return false;
    }

    textures.insert({name, new Texture(shaders[shaderName], width, height)});
    printf("Loaded texture '%s' with shader '%s'\n", name.c_str(),
           shaderName.c_str());
    return true;
}

Shader *ResourceManager::getShader(string name)
{
    auto it = shaders.find(name);

    if (it == shaders.end())
    {
        return nullptr;
    }

    return it->second;
}

Texture *ResourceManager::getTexture(string name)
{
    auto it = textures.find(name);

    if (it == textures.end())
    {
        return nullptr;
    }

    return it->second;
}