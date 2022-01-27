#pragma once

#include "Graphics.h"

#include <string>

class Shader
{
public:
	Shader();
	void load(GLenum type, std::string path);
	GLuint getId();

private:
	GLuint id;
};