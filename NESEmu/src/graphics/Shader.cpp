#include "Shader.h"

#include <assert.h>

#include <fstream>
#include <glm/gtc/type_ptr.hpp>

using std::string;
using std::vector;

void Shader::abandon()
{
    glUseProgram(0);
}

Shader::Shader() : program(0)
{
}

Shader::~Shader()
{
    glDeleteProgram(program);
}

void Shader::load(string fragmentPath, string vertexPath)
{
    program = glCreateProgram();
    GLuint fragment = compile(GL_FRAGMENT_SHADER, fragmentPath);
    GLuint vertex = compile(GL_VERTEX_SHADER, vertexPath);

    assert(program != 0);
    assert(fragment != 0);
    assert(vertex != 0);

    glAttachShader(program, fragment);
    glAttachShader(program, vertex);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        printf("Shader program link error: %s\n", infoLog);
    }

    glDeleteShader(fragment);
    glDeleteShader(vertex);

    GL_ERROR_CHECK();
}

void Shader::use()
{
    glUseProgram(program);
}

GLuint Shader::getId()
{
    return program;
}

GLint Shader::getUniformLocation(string name)
{
    GLint location = glGetUniformLocation(program, name.c_str());

    assert(location >= 0);
    GL_ERROR_CHECK();

    return location;
}

void Shader::setVector3f(string name, const glm::vec3 &vec)
{
    glUniform3f(getUniformLocation(name), vec.x, vec.y, vec.z);
    GL_ERROR_CHECK();
}

void Shader::setVector3f(string name, vector<GLfloat> &vec)
{
    glUniform3fv(getUniformLocation(name), vec.size(), &vec[0]);
    GL_ERROR_CHECK();
}

void Shader::setVector4f(string name, const glm::vec4 &vec)
{
    glUniform4f(getUniformLocation(name), vec.x, vec.y, vec.z, vec.w);
    GL_ERROR_CHECK();
}

void Shader::setVector4f(string name, vector<GLfloat> &vec)
{
    glUniform4fv(getUniformLocation(name), vec.size(), &vec[0]);
    GL_ERROR_CHECK();
}

void Shader::setMatrix4f(string name, const glm::mat4 &matrix)
{
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE,
                       glm::value_ptr(matrix));
    GL_ERROR_CHECK();
}

GLuint Shader::compile(GLuint type, string path)
{
    GLuint id = glCreateShader(type);
    std::ifstream file(path);

    if (file.is_open())
    {
        string str, line;

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

        if (!success)
        {
            char info[1024];
            glGetShaderInfoLog(id, 1024, NULL, info);
            printf("Shader compilation error for '%s': %s\n\n", path.c_str(),
                   info);
        }
    }

    GL_ERROR_CHECK();

    file.close();
    return id;
}