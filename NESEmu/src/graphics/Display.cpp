#include "Display.h"

Display::Display()
{
    window = nullptr;
    shouldTerminate = false;
}

bool Display::init()
{
    if (!glfwInit())
    {
        return false;
    }

    window = glfwCreateWindow(640, 480, "Test", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return false;
    }

    return true;
}

void Display::loop()
{
    while (!glfwWindowShouldClose(window) && !shouldTerminate)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

void Display::terminate()
{
    shouldTerminate = true;
}