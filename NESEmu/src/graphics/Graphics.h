#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>

GLenum _glErrorCheck(const char *file, int line);
void APIENTRY _glDebugOutput(GLenum source, GLenum type, GLuint id,
                             GLenum severity, GLsizei length,
                             const char *message, const void *userParam);

#ifdef NDEBUG
#define GL_ERROR_CHECK() ((void)0)
#else
// #define OPENGL_DEBUG
#define GL_ERROR_CHECK() _glErrorCheck(__FILE__, __LINE__)
#endif
