#include "NES.h"
#include "../graphics/windows/DemoWindow.h"
#include "../graphics/windows/DebugWindow.h"
#include "../graphics/windows/MemoryViewWindow.h"
#include "../graphics/windows/PPUDebugWindow.h"
#include "../graphics/Texture.h"

#include <stdio.h>

const char *WINDOW_TITLE = "nesemu";
static Texture patternTable(128, 128);

static void glfwErrorCallback(int error, const char *desc)
{
    printf("GLFW error: %i %s\n", error, desc);
}

NES::NES(): cpu(memory), ppu(memory)
{
    windowWidth = 1280;
    windowHeight = 720;
    running = false;
    shouldShutdown = false;
    window = nullptr;
    renderingScale = 2.0f;
    emulationSpeed = 1.0;
}

void NES::load(std::string path)
{
	rom.load(path);
	rom.map(memory, cpu, ppu);

    printf("Loaded ROM: %s\n", path.c_str());
	printf("\tMapper: %u\n", rom.getMapperID());
	printf("\tPRG ROM size: %u banks -> %u bytes\n", rom.header.prgBanks, rom.header.prgBanks * 0x4000);
    printf("\tCHR ROM size: %u banks -> %u bytes\n", rom.header.chrBanks, rom.header.chrBanks * 0x2000);

    cpu.reset();
}

bool NES::init()
{
    // Init GLFW
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
    {
        return false;
    }

    // TODO: Configure window to use OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Create window
    window = glfwCreateWindow(windowWidth, windowHeight, WINDOW_TITLE, NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        printf("Failed to init GLFW");
        return false;
    }

    glfwMakeContextCurrent(window);

    // Disable VSync
    glfwSwapInterval(0);

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
    ImGui_ImplOpenGL3_Init(); // "#version 330 core"

    // Init drawables
    drawables.push_back(new DemoWindow());
    drawables.push_back(new DebugWindow(*this, cpu));
    drawables.push_back(new MemoryViewWindow(memory));
    drawables.push_back(new PPUDebugWindow(ppu));

    return true;
}

void NES::run()
{
    const double targetFps = 1.0 / 60.0;
    double lastUpdateTime = 0;  // number of seconds since the last loop
    double lastFrameTime = 0;   // number of seconds since the last frame
    const double cyclesPerFrame = 1790000 / 60; // Amount of CPU cycles needed per UI frame

    printf("Running display at %.2lfHz, with %.2lf CPU cycles per frame\n", 1 / targetFps, cyclesPerFrame);

    patternTable.load(ppu, 0x1000);

    // Draw window and poll events
    while (!glfwWindowShouldClose(window) && !shouldShutdown)
    {
        double now = glfwGetTime();
        double deltaTime = now - lastUpdateTime;
        double modifiedCyclesPerFrame = cyclesPerFrame * emulationSpeed;

        // Poll events
        glfwPollEvents();
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        // Update UI at 60Hz
        if (now - lastFrameTime >= targetFps)
        {
            // Start new ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Update OpenGL
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glViewport(0, 0, width, height);
            glOrtho(0, width, height, 0, -1, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            // Draw graphics from PPU
            draw();

            // Draw all drawable components
            for (IDrawable *drawable : drawables)
            {
                drawable->update();
                drawable->draw();
            }

            // Render ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Swap buffers and finish frame
            glfwSwapBuffers(window);
            lastFrameTime = now;
        }

        // Update NES components independently of UI
        for (int i = 0; running && i < (int)modifiedCyclesPerFrame; i++)
        {
            step();
        }

        lastUpdateTime = now;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    shutdown();
}

void NES::step()
{
    // 1 CPU cycle = 3 PPU cycles
    cpu.step();
    
    // TODO: Improve performance in the future. Fast-forward PPU when relevant registers update
    ppu.step();
    ppu.step();
    ppu.step();
}

void NES::draw()
{
    glEnable(GL_TEXTURE_2D);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // TEST: Drawing nametable 0 using pattern table 1
    uint16_t start = 0x2000;
    float tileSize = 8 * renderingScale;
    float offsetX = (width - tileSize * 32) / 2;
    float offsetY = (height - tileSize * 30) / 2;

    for (uint16_t r = 0; r < 30; r++)
    {
        for (uint16_t c = 0; c < 32; c++)
        {
            uint16_t offset = r * 32 + c;
            uint8_t index = ppu.readMemory(start + offset);
            float rTex = std::floor(index / 16 * 8);
            float cTex = std::floor(index % 16 * 8);

            Vec2 pos(offsetX + c * tileSize, offsetY + r * tileSize);
            Vec2 texPos(cTex, rTex);
            patternTable.draw(pos, Vec2(tileSize, tileSize), texPos, texPos + Vec2(8, 8));
        }
    }

    glDisable(GL_TEXTURE_2D);
}

void NES::shutdown()
{
    shouldShutdown = true;
}

void NES::setRunning(bool running)
{
    this->running = running;
}

bool NES::getRunning() const
{
    return running;
}

void NES::loadDebugMode()
{
    load("..\\roms\\nestest.nes");
    cpu.setPC(0xC000);
    printf("Set PC to 0xC000 for nestest automation mode\n");
}

void NES::setRenderingScale(float scale)
{
    renderingScale = scale;
}

void NES::setEmulationSpeed(double speed)
{
    emulationSpeed = speed;
}

ROM& NES::getRom()
{
    return rom;
}
