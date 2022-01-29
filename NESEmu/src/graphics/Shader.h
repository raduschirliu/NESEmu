#pragma once

#include "Graphics.h"

#include <string>
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

	void setVector3f(std::string name, const glm::vec3 &vector);
	void setVector3f(std::string name, GLsizei count, const GLfloat *arr);
	void setMatrix4f(std::string name, const glm::mat4 &matrix);

private:
	GLuint program;

	GLuint compile(GLuint type, std::string path);
};