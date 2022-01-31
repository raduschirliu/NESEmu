#pragma once

#include "Graphics.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>

class Shader
{
public:
	static void abandon();

	Shader();
	~Shader();

	void load(std::string fragmentPath, std::string vertexPath);
	void use();
	GLuint getId();
	GLint getUniformLocation(std::string name);

	void setVector3f(std::string name, const glm::vec3 &vec);
	void setVector3f(std::string name, std::vector<GLfloat> &vec);
	void setVector4f(std::string name, std::vector<GLfloat> &vec);
	void setMatrix4f(std::string name, const glm::mat4 &matrix);

private:
	GLuint program;

	GLuint compile(GLuint type, std::string path);
};