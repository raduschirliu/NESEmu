#include "Shader.h"

#include <fstream>

Shader::Shader() : id(0)
{

}

void Shader::load(GLenum type, std::string path)
{
	id = glCreateShader(type);
	std::ifstream file(path);

	if (file.is_open())
	{
		std::string str;
		std::string line;

		while (!file.eof())
		{
			std::getline(file, line);
			str += line;
			str.push_back('\n');
		}

		const char *cStr = str.c_str();
		glShaderSource(id, 1, &cStr, NULL);
		glCompileShader(id);

		int success;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);

		if (success)
		{
			printf("Loaded shader: %s\n", path.c_str());
		}
		else
		{
			char info[512];
			glGetShaderInfoLog(id, 512, NULL, info);
			printf("Shader compilation error for '%s': %s\n\n", path.c_str(), info);
		}
	}

	file.close();
}

GLuint Shader::getId()
{
	return id;
}