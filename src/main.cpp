// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "window.hpp"
#include "gfx/graphics.hpp"
#include "gfx/shader.hpp"
#include "gfx/font.hpp"
#include "gfx/text_shaper.hpp"
#include "gfx/text_renderer.hpp"
#include "loaders/ktx.hpp"

#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <csignal>
#include <cmath>
#include <vector>
#include <array>
#include <filesystem>
#include <memory>
#include <algorithm>
#include <string>

extern int currentAnimation;

static std::unique_ptr<Window> g_window;

static void signal_handler(int signal)
{
    spdlog::info("Received signal: {}", signal);
    if (g_window)
    {
        g_window->close();
    }
}

int main()
{
    spdlog::set_level(spdlog::level::info);

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try
    {
        g_window = std::make_unique<Window>();

        float zoomLevel = 1.0f;
        glm::vec2 cameraPosition{0.0f, 0.0f};
        bool isPanning = false;
        glm::vec2 lastMousePos{0.0f, 0.0f};

        g_window->setScrollCallback([&zoomLevel](double /*xoffset*/, double yoffset)
        {
            if (yoffset > 0)
            {
                zoomLevel *= 0.9f;
            }
            else if (yoffset < 0)
            {
                zoomLevel *= 1.1f;
            }
            zoomLevel = std::clamp(zoomLevel, 0.1f, 10.0f);
            spdlog::info("Camera zoom: {:.2f}", zoomLevel);
        });

        g_window->setMouseButtonCallback([&](int button, int action, int /*mods*/)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE || button == GLFW_MOUSE_BUTTON_RIGHT)
            {
                if (action == GLFW_PRESS)
                {
                    isPanning = true;
                }
                else if (action == GLFW_RELEASE)
                {
                    isPanning = false;
                }
            }
        });

        g_window->setCursorPosCallback([&](double xpos, double ypos)
        {
            const glm::vec2 currentPos{static_cast<float>(xpos), static_cast<float>(ypos)};
            if (isPanning)
            {
                const glm::vec2 delta = currentPos - lastMousePos;
                const auto winSize = g_window->getWindowSize();
                const float aspect = winSize.x / winSize.y;

                const float worldWidth = 2.0f * zoomLevel * aspect;
                const float worldHeight = 2.0f * zoomLevel;

                cameraPosition.x -= delta.x * (worldWidth / winSize.x);
                cameraPosition.y += delta.y * (worldHeight / winSize.y);
            }
            lastMousePos = currentPos;
        });

        g_window->setKeyCallback([&](int key, int /*scancode*/, int action, int /*mods*/)
        {
            if (key == GLFW_KEY_R && action == GLFW_PRESS)
            {
                cameraPosition = {0.0f, 0.0f};
                zoomLevel = 1.0f;
                spdlog::info("Camera reset to position (0, 0) and zoom 1.0");
            }
        });

        const auto binaryPath = std::filesystem::read_symlink("/proc/self/exe");
        const auto basePath = binaryPath.parent_path();

        // 1. Load FreeType & HarfBuzz font
        const auto fontPath = basePath / "assets" / "font.ttf";
        bgl::Font font(fontPath.string(), 36);

        // 2. Load animation textures using polymorphic ITextureLoader
        const std::array fileNames{
            basePath / "assets" / "idle.ktx2",
            basePath / "assets" / "walk.ktx2",
            basePath / "assets" / "jump.ktx2",
            basePath / "assets" / "run.ktx2",
            basePath / "assets" / "slide.ktx2",
            basePath / "assets" / "dead.ktx2"};

        std::vector<std::unique_ptr<bgl::ITextureLoader>> loaders;
        loaders.reserve(fileNames.size());
        for (const auto &file : fileNames)
        {
            loaders.push_back(std::make_unique<bgl::ktx::Loader>(file, KTX_TTF_BC3_RGBA));
        }

        std::vector<GLuint> textureIDs(loaders.size());
        for (size_t i = 0; i < loaders.size(); ++i)
        {
            textureIDs[i] = loaders[i]->upload();
        }
        loaders.clear();

        // 3. Load SPIR-V shaders from assets directory
        const auto mainVsSpv = bgl::LoadSPIRVShaderFromFile(basePath / "assets" / "main.vert.spv");
        const auto mainFsSpv = bgl::LoadSPIRVShaderFromFile(basePath / "assets" / "main.frag.spv");
        const GLuint mainProgram = bgl::CreateShaderProgramFromSPIRV(mainVsSpv, mainFsSpv);

        const auto textVsSpv = bgl::LoadSPIRVShaderFromFile(basePath / "assets" / "text.vert.spv");
        const auto textFsSpv = bgl::LoadSPIRVShaderFromFile(basePath / "assets" / "text.frag.spv");
        const GLuint textProgram = bgl::CreateShaderProgramFromSPIRV(textVsSpv, textFsSpv);

        // 4. Create meshes
        bgl::QuadMesh spriteQuad = bgl::create2DQuad();
        bgl::QuadMesh overlayQuad = bgl::createOverlayQuad();

        // Variables for FPS calculation and text caching
        double lastFpsTime = 0.0;
        int frameCounter = 0;
        int currentFps = 60;
        std::string lastHudText;
        std::optional<bgl::TextTexture> hudTexture;

        // 5. Register render callback
        g_window->setRenderCallback([&](double time)
        {
            frameCounter++;
            if (time - lastFpsTime >= 0.2)
            {
                currentFps = static_cast<int>(std::round(frameCounter / (time - lastFpsTime)));
                frameCounter = 0;
                lastFpsTime = time;
            }

            const auto count = static_cast<int>(textureIDs.size());
            const auto currentTexture = static_cast<size_t>(((currentAnimation % count) + count) % count);
            const std::string currentFilename = fileNames[currentTexture].filename().string();
            const std::string currentHudText = fmt::format("FPS: {}; {}", currentFps, currentFilename);

            if (currentHudText != lastHudText || !hudTexture.has_value())
            {
                lastHudText = currentHudText;
                hudTexture = bgl::TextRenderer::RenderToTexture(
                    font,
                    currentHudText,
                    {255, 255, 255, 255},  // White text color
                    {0, 0, 0, 0},          // Transparent background
                    4                      // Padding
                );
            }

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            const auto c = (static_cast<int>(time) % 10) / 10.0f;
            glClearColor(c * 0.2f, c * 0.1f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            GLint layers = 0;
            glGetTextureLevelParameteriv(textureIDs[currentTexture], 0, GL_TEXTURE_DEPTH, &layers);

            const auto num_frames = layers > 0 ? layers : 1;
            constexpr double target_fps = 24.0;

            const double totalFrames = time * target_fps;
            const int currentFrame = static_cast<int>(std::floor(totalFrames)) % num_frames;
            const float tweenFactor = static_cast<float>(totalFrames - std::floor(totalFrames));

            const auto winSize = g_window->getWindowSize();
            const float aspect = winSize.x / winSize.y;
            const glm::mat4 projection = glm::ortho(
                cameraPosition.x - zoomLevel * aspect,
                cameraPosition.x + zoomLevel * aspect,
                cameraPosition.y - zoomLevel,
                cameraPosition.y + zoomLevel,
                -1.0f, 1.0f
            );

            // Render background sprite animation quad
            bgl::renderQuad(spriteQuad, textureIDs[currentTexture], mainProgram, currentFrame, tweenFactor, projection);

            // Render top-left FPS & VRAM text texture overlay
            if (hudTexture && hudTexture->IsValid())
            {
                bgl::renderTextOverlay(
                    overlayQuad,
                    hudTexture->GetHandle(),
                    textProgram,
                    hudTexture->GetWidth(),
                    hudTexture->GetHeight(),
                    static_cast<uint32_t>(winSize.x),
                    static_cast<uint32_t>(winSize.y),
                    15.0f,
                    15.0f
                );
            }
        });

        g_window->run();

        // GPU resources cleanup
        hudTexture.reset();

        if (!textureIDs.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(textureIDs.size()), textureIDs.data());
            spdlog::info("Released {} OpenGL textures from VRAM", textureIDs.size());
            textureIDs.clear();
        }

        if (mainProgram != 0)
        {
            glDeleteProgram(mainProgram);
        }
        if (textProgram != 0)
        {
            glDeleteProgram(textProgram);
        }

        bgl::destroyQuadMesh(spriteQuad);
        bgl::destroyQuadMesh(overlayQuad);
        spdlog::info("Released VAO/VBO/IBO buffers and shader programs");
    }
    catch (const std::exception &e)
    {
        spdlog::error("{}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}