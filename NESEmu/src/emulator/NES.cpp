#include "NES.h"
#include "../graphics/windows/DemoWindow.h"
#include "../graphics/windows/DebugWindow.h"
#include "../graphics/windows/MemoryViewWindow.h"
#include "../graphics/windows/PPUDebugWindow.h"
#include "../graphics/Texture.h"
#include "../graphics/Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <stdio.h>

const char *WINDOW_TITLE = "nesemu";
static Texture patternTable(128, 128);

static GLuint program;
static Shader fragment, vertex;

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
    ImGui_ImplOpenGL3_Init("#version 330 core");

    program = glCreateProgram();
    fragment.load(GL_FRAGMENT_SHADER, "shader.frag");
    vertex.load(GL_VERTEX_SHADER, "shader.vert");

    glAttachShader(program, fragment.getId());
    glAttachShader(program, vertex.getId());
    glLinkProgram(program);

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

// TODO: temp
std::vector<float> normalizePalette(std::vector<PPU::Color> palette)
{
    std::vector<float> normalized;

    for (auto color : palette)
    {
        normalized.push_back(color.r / 255.0f);
        normalized.push_back(color.g / 255.0f);
        normalized.push_back(color.b / 255.0f);
    }

    return normalized;
}

void NES::draw()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // TODO: Determine the correct nametable to be drawing
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
            
            glUseProgram(program);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(pos.x, pos.y, 0.0f));
            model = glm::scale(model, glm::vec3(tileSize, tileSize, 1.0f));
            glm::mat4 projection = glm::ortho<float>(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

            uint16_t attCol = c / 4;
            uint16_t attRow = r / 4;
            uint16_t attTableAddress = 0x23C0 + attRow * 8 + attCol;
            uint8_t attByte = ppu.readMemory(attTableAddress);
            uint8_t shift = 0;

            if (c % 4 >= 2)
            {
                // Right
                shift += 2;
            }

            if (r % 4 >= 2)
            {
                // Bottom
                shift += 4;
            }

            // TODO: Attribute table may have issues with edge cases? Donkey Kong's body has white lines
            uint8_t attBits = (attByte & (0b11 << shift)) >> shift;
            uint16_t addresses[] = { 0x3F01, 0x3F05, 0x3F09, 0x3F0D };
            uint16_t paletteAddress = addresses[attBits];
            auto ppuPalette = ppu.getPalette(paletteAddress);

            std::vector<float> palette = normalizePalette(ppuPalette);

            glUniform3fv(glGetUniformLocation(program, "palette"), 4, &palette[0]);
            glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            patternTable.draw(pos, Vec2(tileSize, tileSize), texPos, texPos + Vec2(8, 8));
            glUseProgram(0);
            
        }
    }
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
