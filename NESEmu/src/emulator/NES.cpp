#include "NES.h"
#include "../graphics/windows/DemoWindow.h"
#include "../graphics/windows/DebugWindow.h"
#include "../graphics/windows/MemoryViewWindow.h"
#include "../graphics/windows/PPUDebugWindow.h"
#include "../graphics/Texture.h"
#include "../graphics/Shader.h"
#include "../graphics/ResourceManager.h"

#include <glm/glm.hpp>

#include <iostream>
#include <stdio.h>

static const char *WINDOW_TITLE = "nesemu";

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

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#ifdef OPENGL_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

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

    printf("Loaded OpenGL version %s\n", glGetString(GL_VERSION));

    // TODO: Move to graphics.cpp / graphics.h
#ifdef OPENGL_DEBUG
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(_glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        printf("Enabled OpenGL debug context\n");
    }
#endif

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    GL_ERROR_CHECK();

    ResourceManager::loadShader("pattern_shader", "shader.frag", "shader.vert");
    ResourceManager::loadTexture("pattern_left", "pattern_shader", 128, 128);
    ResourceManager::loadTexture("pattern_right", "pattern_shader", 128, 128);

    GL_ERROR_CHECK();

    // Init drawables
    drawables.push_back(new DemoWindow());
    drawables.push_back(new DebugWindow(*this, cpu));
    drawables.push_back(new MemoryViewWindow(memory));
    drawables.push_back(new PPUDebugWindow(ppu));

    GL_ERROR_CHECK();

    return true;
}

void NES::run()
{
    const double targetFps = 1.0 / 60.0;
    double lastUpdateTime = 0;  // number of seconds since the last loop
    double lastFrameTime = 0;   // number of seconds since the last frame
    const double cyclesPerFrame = 1790000 / 60; // Amount of CPU cycles needed per UI frame

    printf("Running display at %.2lfHz, with %.2lf CPU cycles per frame\n", 1 / targetFps, cyclesPerFrame);

    // TODO: Create a PPU callback for loading pattern tables
    Texture *leftPatternTable = ResourceManager::getTexture("pattern_left");
    Texture *rightPatternTable = ResourceManager::getTexture("pattern_right");
    leftPatternTable->load(ppu, 0x0000);
    rightPatternTable->load(ppu, 0x1000);

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
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT);

            // Draw graphics from PPU
            drawBackground();
            drawSprites();

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

void NES::drawBackground()
{
    // TODO: Draw background color separately to allow sprites to appear behind background

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    uint16_t start = ppu.getActiveNametableAddress();
    float tileSize = 8 * renderingScale;
    float offsetX = (width - tileSize * 32) / 2;
    float offsetY = (height - tileSize * 30) / 2;

    uint8_t nametable = ppu.getRegisters()->ctrl.baseNametable;
    Texture *patternTable = ResourceManager::getTexture(
        ppu.getActiveBgPatternTableAddress() == 0x0000 ? "pattern_left" : "pattern_right");
    uint16_t bgPaletteAddresses[] = { 0x3F01, 0x3F05, 0x3F09, 0x3F0D };

    for (uint32_t r = 0; r < 30; r++)
    {
        for (uint32_t c = 0; c < 32; c++)
        {
            uint16_t offset = r * 32 + c;
            uint8_t index = ppu.readMemory(start + offset);
            float cTex = std::floor(index % 16 * 8);
            float rTex = std::floor(index / 16 * 8);

            glm::vec2 pos(offsetX + c * tileSize, offsetY + r * tileSize);
            glm::vec2 size(tileSize, tileSize);
            glm::vec2 texPos(cTex, rTex);

            uint8_t paletteTableIndex = ppu.getNametableEntryPalette(nametable, offset);
            uint16_t paletteAddress = bgPaletteAddresses[paletteTableIndex];
            auto palette = ppu.getPalette(paletteAddress);
            patternTable->draw(pos, size, texPos, texPos + glm::vec2(8, 8), palette);
        }
    }
}

void NES::drawSprites()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    float tileSize = 8 * renderingScale;
    float offsetX = (width - tileSize * 32) / 2;
    float offsetY = (height - tileSize * 30) / 2;
    Texture *patternTable = ResourceManager::getTexture(
        ppu.getActiveSpritePatternTableAddress() == 0x0000 ? "pattern_left" : "pattern_right");

    uint16_t paletteAddresses[] = { 0x3F11, 0x3F15, 0x3F19, 0x3F1D };

    // 64 sprites in Oam to draw. Sprites with lower address are drawn on top
    for (int i = 63; i >= 0; i--)
    {
        PPU::OamSprite *sprite = ppu.getOamSprite(i * sizeof(PPU::OamSprite));
        uint16_t paletteAddress = paletteAddresses[sprite->attributes.palette];
        auto palette = ppu.getPalette(paletteAddress);

        // TODO: This may be incorrect, Mario doesn't display in DK
        if (sprite->xPos < 0 || sprite->yPos < 0)
        {
            continue;
        }

        // Only draw sprites that are supposed to be in front of the background
        if (sprite->attributes.priority != 0)
        {
            continue;
        }

        glm::vec2 pos(offsetX + sprite->xPos * renderingScale, offsetY + sprite->yPos * renderingScale);
        glm::vec2 size(tileSize, tileSize);

        float cTex = std::floor(sprite->tileIndex % 16 * 8);
        float rTex = std::floor(sprite->tileIndex / 16 * 8);
        glm::vec2 texPos(cTex, rTex);

        patternTable->draw(pos, size, texPos, texPos + glm::vec2(8, 8), palette);
    }
}
