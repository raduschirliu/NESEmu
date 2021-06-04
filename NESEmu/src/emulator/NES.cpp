#include "NES.h"
#include "../graphics/windows/DemoWindow.h"
#include "../graphics/windows/CpuWindow.h"

#include <stdio.h>

const char *WINDOW_TITLE = "nesemu";

static void glfwErrorCallback(int error, const char *desc)
{
    printf("GLFW error: %i %s\n", error, desc);
}

NES::NES(): cpu(&memory)
{
    windowWidth = 1280;
    windowHeight = 720;
    shouldShutdown = false;
    window = nullptr;
}

void NES::load(std::string path)
{
	rom.load(path);
	rom.map(&memory, &ppu);

	printf("Mapper: %u\n", rom.getMapperID());
	printf("ROM size: %u\n", rom.header.prgBanks * 0x4000);
}

bool NES::init()
{
    // Init GLFW
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
    {
        return false;
    }

    // Configure window to use OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Create window
    window = glfwCreateWindow(windowWidth, windowHeight, WINDOW_TITLE, NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        printf("Failed to init GLFW");
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to init GLEW");
        return false;
    }

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Init drawables
    drawables.push_back(new DemoWindow());
    drawables.push_back(new CpuWindow(cpu));

    return true;
}

void NES::run()
{
    // Draw window and poll events
    while (!glfwWindowShouldClose(window) && !shouldShutdown)
    {
        // Poll events
        glfwPollEvents();
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        // Start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Draw all drawable components
        for (IDrawable *drawable : drawables)
        {
            drawable->update();
            drawable->draw();
        }

        // Update OpenGL
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and finish frame
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    shutdown();
}

void NES::shutdown()
{
    shouldShutdown = true;
}

void NES::loadDebugMode()
{
    load("..\\roms\\nestest.nes");
    cpu.setPC(0xC000);
}