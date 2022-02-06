#include "NES.h"
#include "../graphics/windows/DemoWindow.h"
#include "../graphics/windows/DebugWindow.h"
#include "../graphics/windows/MemoryViewWindow.h"
#include "../graphics/windows/PPUDebugWindow.h"
#include "../graphics/windows/InputDebugWindow.h"
#include "../graphics/windows/CartridgeDebugWindow.h"
#include "../graphics/Texture.h"
#include "../graphics/Shader.h"
#include "../graphics/ResourceManager.h"
#include "../util/Input.h"

#include <iostream>
#include <stdio.h>

using std::floor;

static const char *WINDOW_TITLE = "nesemu";

// Depths of various rendering layers
static constexpr float BACKGROUND_COLOR_DEPTH = -5.0f;
static constexpr float BACKGROUND_SPRITE_DEPTH = -1.0f;
static constexpr float BACKGROUND_TILE_DEPTH = 0.0f;
static constexpr float FOREGROUND_SPRITE_DEPTH = 1.0f;

static void glfwErrorCallback(int error, const char *desc)
{
    printf("GLFW error: %i %s\n", error, desc);
}

void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{

}

NES::NES(): cpu(bus), ppu(bus), controller(bus, Bus::JOY1)
{
    // TODO: Use initializer list
    windowWidth = 1280;
    windowHeight = 720;
    viewportWidth = 0;
    viewportHeight = 0;
    running = false;
    shouldShutdown = false;
    window = nullptr;
    renderingScale = 2.0f;
    emulationSpeed = 1.0;
}

void NES::load(std::string path)
{
    cartridge.load(path);
    bus.setMapper(cartridge.getMapper());
    ppu.setMapper(cartridge.getMapper());

    cpu.reset();
    ppu.reset();
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

    Input::init(window);

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

    int patternTableSize = PPU::PATTERN_TABLE_SIZE * PPU::TILE_SIZE;
    ResourceManager::loadShader("pattern_shader", "shader.frag", "shader.vert");
    ResourceManager::loadTexture("pattern_left", "pattern_shader", patternTableSize, patternTableSize);
    ResourceManager::loadTexture("pattern_right", "pattern_shader", patternTableSize, patternTableSize);

    GL_ERROR_CHECK();

    // Init drawables
    drawables.push_back(new DemoWindow());
    drawables.push_back(new DebugWindow(*this, cpu));
    drawables.push_back(new MemoryViewWindow(bus));
    drawables.push_back(new PPUDebugWindow(*this, ppu));
    drawables.push_back(new CartridgeDebugWindow(cartridge));
    drawables.push_back(new InputDebugWindow(controller));

    GL_ERROR_CHECK();

    Input::registerKeyMap("joy1",
        {
            { GLFW_KEY_A, Controller::Button::B },
            { GLFW_KEY_S, Controller::Button::A },
            { GLFW_KEY_Q, Controller::Button::SELECT },
            { GLFW_KEY_W, Controller::Button::START },
            { GLFW_KEY_UP, Controller::Button::UP },
            { GLFW_KEY_DOWN, Controller::Button::DOWN },
            { GLFW_KEY_LEFT, Controller::Button::LEFT },
            { GLFW_KEY_RIGHT, Controller::Button::RIGHT },
        });

    return true;
}

void NES::run()
{
    const double targetFps = 1.0 / 60.0;
    double lastUpdateTime = 0;  // number of seconds since the last loop
    double lastFrameTime = 0;   // number of seconds since the last frame
    const double cyclesPerFrame = 1790000 / 60; // Amount of CPU cycles needed per UI frame

    printf("Running display at %.2lfHz, with %.2lf CPU cycles per frame\n", 1 / targetFps, cyclesPerFrame);

    // TODO: Create a PPU callback for loading/updating pattern tables
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
            glfwGetFramebufferSize(window, &viewportWidth, &viewportHeight);
            glViewport(0, 0, viewportWidth, viewportHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Draw graphics from PPU
            drawBackground();
            drawSprites();

            // Draw all drawable components
            for (IDrawable *drawable : drawables)
            {
                if (!drawable->isActive())
                {
                    continue;
                }

                drawable->update();

                if (drawable->isVisible())
                {
                    drawable->draw();
                }
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

    if (controller.isPolling())
    {
        Controller::ButtonStates keyMap = Input::getKeyMap("joy1");
        controller.setButtonStates(keyMap);
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

float NES::getRenderingScale()
{
    return renderingScale;
}

void NES::setEmulationSpeed(double speed)
{
    emulationSpeed = speed;
}

Cartridge & NES::getCartridge()
{
    return cartridge;
}

float NES::getTileSize()
{
    return PPU::TILE_SIZE * renderingScale;
}

glm::vec2 NES::getGraphicsOffset()
{
    float tileSize = getTileSize();

    return glm::vec2(
        (viewportWidth - tileSize * PPU::NAMETABLE_COLS) / 2,
        (viewportHeight - tileSize * PPU::NAMETABLE_ROWS) / 2
    );
}

void NES::drawBackground()
{
    uint16_t start = ppu.getActiveNametableAddress();
    float tileSize = getTileSize();
    glm::vec2 offset = getGraphicsOffset();

    uint8_t nametable = ppu.getRegisters()->ctrl.baseNametable;
    Texture *patternTable = ResourceManager::getTexture(
        ppu.getActiveBgPatternTableAddress() == 0x0000 ? "pattern_left" : "pattern_right");
    uint16_t bgColorAddress = 0x3F00;
	uint16_t bgPaletteAddresses[] = { 0x3F01, 0x3F05, 0x3F09, 0x3F0D };
    std::vector<PPU::Color> grayscalePalette = { { 0, 0, 0, 0 }, { 50, 50, 50, 255 }, { 100, 100, 100, 255 }, { 200, 200, 200, 255 } };

    // Solid background color
    {
        glm::vec3 pos(offset.x, offset.y, BACKGROUND_COLOR_DEPTH);
        glm::vec2 size(PPU::NAMETABLE_COLS * tileSize, PPU::NAMETABLE_ROWS * tileSize);
        glm::vec2 texPos(0.0f, 0.0f);
        glm::vec2 texEndPos(patternTable->getWidth(), patternTable->getHeight());

        // TODO: Draw this without needlessly using texture
        PPU::Color bgColor = ppu.getPalette(bgColorAddress)[0];
        std::vector<PPU::Color> palette(4, bgColor);

        patternTable->draw(pos, size, texPos, texEndPos, palette);
    }

    // Nametable background tiles
    if (ppu.getRegisters()->mask.showBg)
    {
        for (uint32_t r = 0; r < PPU::NAMETABLE_ROWS; r++)
        {
            if (!ppu.getRegisters()->mask.showLeftmostBg)
            {
                continue;
            }

            for (uint32_t c = 0; c < PPU::NAMETABLE_COLS; c++)
            {
                uint16_t nametableIndex = r * PPU::NAMETABLE_COLS + c;
                uint8_t patternIndex = ppu.readMemory(start + nametableIndex);
                float cTex = floor(patternIndex % PPU::PATTERN_TABLE_SIZE * PPU::TILE_SIZE);
                float rTex = floor(patternIndex / PPU::PATTERN_TABLE_SIZE * PPU::TILE_SIZE);

                glm::vec3 pos(offset.x + c * tileSize, offset.y + r * tileSize, BACKGROUND_TILE_DEPTH);
                glm::vec2 size(tileSize, tileSize);
                glm::vec2 texPos(cTex, rTex);
                glm::vec2 texPosEnd(cTex + PPU::TILE_SIZE, rTex + PPU::TILE_SIZE);

                uint8_t paletteTableIndex = ppu.getNametableEntryPalette(nametable, nametableIndex);
                uint16_t paletteAddress = bgPaletteAddresses[paletteTableIndex];
                auto palette = ppu.getPalette(paletteAddress);

                if (ppu.getRegisters()->mask.grayscale)
                {
                    palette = grayscalePalette;
                }

                patternTable->draw(pos, size, texPos, texPosEnd, palette);
            }
        }
    }
}

void NES::drawSprites()
{
    if (!ppu.getRegisters()->mask.showSprites)
    {
        return;
    }

    float tileSize = getTileSize();
    glm::vec2 offset = getGraphicsOffset();
    Texture *patternTable = ResourceManager::getTexture(
        ppu.getActiveSpritePatternTableAddress() == 0x0000 ? "pattern_left" : "pattern_right");

    uint16_t paletteAddresses[] = { 0x3F11, 0x3F15, 0x3F19, 0x3F1D };
    uint16_t nesWidth = PPU::NAMETABLE_COLS * PPU::TILE_SIZE;
    uint16_t nesHeight = PPU::NAMETABLE_ROWS * PPU::TILE_SIZE;
    std::vector<PPU::Color> grayscalePalette = { { 0, 0, 0, 0 }, { 50, 50, 50, 255 }, { 100, 100, 100, 255 }, { 200, 200, 200, 255 } };

    // 64 sprites in Oam to draw. Sprites with lower address are drawn on top
    for (int i = PPU::OAM_SIZE - 1; i >= 0; i--)
    {
        PPU::OamSprite *sprite = ppu.getOamSprite(i * sizeof(PPU::OamSprite));
        uint16_t paletteAddress = paletteAddresses[sprite->attributes.palette];
        auto palette = ppu.getPalette(paletteAddress);

        if (ppu.getRegisters()->mask.grayscale)
        {
            palette = grayscalePalette;
        }

        if (sprite->xPos > nesWidth || sprite->yPos > nesHeight)
        {
            continue;
        }

        if (!ppu.getRegisters()->mask.showLeftmostSprite && sprite->xPos < 8)
        {
            continue;
        }

        // Screen coordinates
        glm::vec3 pos(
            offset.x + sprite->xPos * renderingScale,
            offset.y + sprite->yPos * renderingScale,
            sprite->attributes.priority == 0 ? FOREGROUND_SPRITE_DEPTH : BACKGROUND_SPRITE_DEPTH
        );
        glm::vec2 size(tileSize, tileSize);

        // Texture coordinates
        float cTex = floor(sprite->tileIndex % PPU::PATTERN_TABLE_SIZE * PPU::TILE_SIZE);
        float rTex = floor(sprite->tileIndex / PPU::PATTERN_TABLE_SIZE * PPU::TILE_SIZE);
        glm::vec2 texPos(cTex, rTex);
        glm::vec2 texPosEnd(cTex + PPU::TILE_SIZE, rTex + PPU::TILE_SIZE);

        // Flip sprites
        if (sprite->attributes.flipHorizontal)
        {
            texPos.x = cTex + PPU::TILE_SIZE;
            texPosEnd.x = cTex;
        }

        if (sprite->attributes.flipVertical)
        {
            texPos.y = rTex + PPU::TILE_SIZE;
            texPosEnd.y = rTex;
        }

        patternTable->draw(pos, size, texPos, texPosEnd, palette);
    }
}
