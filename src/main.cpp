// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "Window.hpp"
#include "gfx/Graphics.hpp"
#include "gfx/Font.hpp"
#include "gfx/Text_shaper.hpp"
#include "gfx/Text_renderer.hpp"
#include "io/KtxLoader.hpp"
#include "io/PngLoader.hpp"
#include "io/ShaderLoader.hpp"
#include "Character.hpp"

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

// Main Character instance
static std::shared_ptr<bgl::Character> g_character;

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
            
            if (g_character)
            {
                if ((key == GLFW_KEY_SPACE || key == GLFW_KEY_UP) && action == GLFW_PRESS)
                {
                    g_character->nextAnimation();
                }
                else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
                {
                    g_character->previousAnimation();
                }
            }
        });

        const auto binaryPath = std::filesystem::read_symlink("/proc/self/exe");
        const auto basePath = binaryPath.parent_path();

        // 1. Load FreeType & HarfBuzz font via Fontconfig
        bgl::Font font = bgl::Font::LoadSystemFont("sans-serif", 36);

        // 2. Load animations into Character
        g_character = std::make_shared<bgl::Character>("Hero");
        
        const std::array fileNames{
            std::make_pair("Idle", basePath / "assets" / "idle.ktx2"),
            std::make_pair("Walk", basePath / "assets" / "walk.ktx2"),
            std::make_pair("Jump", basePath / "assets" / "jump.ktx2"),
            std::make_pair("Run", basePath / "assets" / "run.ktx2"),
            std::make_pair("Slide", basePath / "assets" / "slide.ktx2"),
            std::make_pair("Dead", basePath / "assets" / "dead.ktx2")
        };

        for (const auto& [animName, file] : fileNames)
        {
            bgl::io::KtxLoader loader(file, KTX_TTF_BC3_RGBA);
            auto texture = loader.upload();
            
            GLint layers = 0;
            glGetTextureLevelParameteriv(texture->getHandle(), 0, GL_TEXTURE_DEPTH, &layers);
            uint32_t frameCount = static_cast<uint32_t>(layers > 0 ? layers : 1);
            
            g_character->addAnimation(animName, std::move(texture), frameCount);
        }

        // 3. Load SPIR-V shaders from assets directory
        const auto mainVsSpv = bgl::io::LoadSPIRVShaderFromFile(basePath / "assets" / "main.vert.spv");
        const auto mainFsSpv = bgl::io::LoadSPIRVShaderFromFile(basePath / "assets" / "main.frag.spv");
        const GLuint mainProgram = bgl::io::CreateShaderProgramFromSPIRV(mainVsSpv, mainFsSpv);

        const auto textVsSpv = bgl::io::LoadSPIRVShaderFromFile(basePath / "assets" / "text.vert.spv");
        const auto textFsSpv = bgl::io::LoadSPIRVShaderFromFile(basePath / "assets" / "text.frag.spv");
        const GLuint textProgram = bgl::io::CreateShaderProgramFromSPIRV(textVsSpv, textFsSpv);
        
        // Compile Checkered Background Shader (GLSL)
        GLuint bgProgram = 0;
        GLuint snowProgram = 0;
#ifdef BGL_ENABLE_GLSL_LOADER
        const auto bgVsSrc = bgl::io::LoadShaderFromFile(basePath / "src" / "shaders" / "background.vs");
        const auto bgFsSrc = bgl::io::LoadShaderFromFile(basePath / "src" / "shaders" / "background.fs");
        bgProgram = bgl::io::CreateShaderProgramFromGLSL(bgVsSrc, bgFsSrc);

        const auto snowVsSrc = bgl::io::LoadShaderFromFile(basePath / "src" / "shaders" / "snow.vs");
        const auto snowFsSrc = bgl::io::LoadShaderFromFile(basePath / "src" / "shaders" / "snow.fs");
        snowProgram = bgl::io::CreateShaderProgramFromGLSL(snowVsSrc, snowFsSrc);
#endif

        // Load snowflake texture
        std::unique_ptr<bgl::gfx::Texture2DArray> snowTexture;
#ifdef BGL_ENABLE_PNG_LOADER
        bgl::io::PngLoader snowLoader(basePath / "assets" / "snowflake.png");
        snowTexture = snowLoader.upload();
#endif

        // 4. Create meshes
        bgl::QuadMesh spriteQuad = bgl::create2DQuad();
        bgl::QuadMesh overlayQuad = bgl::createOverlayQuad();
        bgl::QuadMesh bgQuad = bgl::create2DQuad();

        // Snow particle instance VBO
        constexpr int NUM_SNOW_PARTICLES = 10000;
        std::vector<glm::vec2> snowOffsets(NUM_SNOW_PARTICLES);
        for (int i = 0; i < NUM_SNOW_PARTICLES; ++i)
        {
            // Random offsets between -10 and 10
            float rx = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 20.0f - 10.0f;
            float ry = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 20.0f - 10.0f;
            snowOffsets[i] = glm::vec2(rx, ry);
        }

        GLuint snowVBO = 0;
        glCreateBuffers(1, &snowVBO);
        glNamedBufferStorage(snowVBO, snowOffsets.size() * sizeof(glm::vec2), snowOffsets.data(), 0);

        // Bind instance VBO to bgQuad (or we could create a new VAO, but reusing bgQuad's VAO is fine if we are careful)
        // Actually, let's create a dedicated snow VAO
        GLuint snowVAO = 0;
        glCreateVertexArrays(1, &snowVAO);
        
        // Base quad vertices
        constexpr std::array<glm::vec2, 4> baseQuad = {
            glm::vec2(-1.0f, -1.0f), glm::vec2(1.0f, -1.0f),
            glm::vec2(1.0f, 1.0f), glm::vec2(-1.0f, 1.0f)
        };
        constexpr std::array<GLuint, 6> baseIndices = {0, 1, 2, 2, 3, 0};
        
        GLuint quadVBO = 0, quadIBO = 0;
        glCreateBuffers(1, &quadVBO);
        glCreateBuffers(1, &quadIBO);
        glNamedBufferStorage(quadVBO, baseQuad.size() * sizeof(glm::vec2), baseQuad.data(), 0);
        glNamedBufferStorage(quadIBO, baseIndices.size() * sizeof(GLuint), baseIndices.data(), 0);

        glVertexArrayVertexBuffer(snowVAO, 0, quadVBO, 0, sizeof(glm::vec2));
        glVertexArrayVertexBuffer(snowVAO, 1, snowVBO, 0, sizeof(glm::vec2));
        glVertexArrayElementBuffer(snowVAO, quadIBO);

        glEnableVertexArrayAttrib(snowVAO, 0);
        glVertexArrayAttribFormat(snowVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(snowVAO, 0, 0);

        glEnableVertexArrayAttrib(snowVAO, 1);
        glVertexArrayAttribFormat(snowVAO, 1, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(snowVAO, 1, 1);
        glVertexArrayBindingDivisor(snowVAO, 1, 1); // Instanced

        // Variables for FPS calculation and text caching
        double lastFpsTime = 0.0;
        int frameCounter = 0;
        int currentFps = 60;
        std::string lastHudText1;
        std::string lastHudText2;
        std::optional<bgl::TextTexture> hudTexture1;
        std::optional<bgl::TextTexture> hudTexture2;

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

            const auto* animState = g_character ? g_character->getCurrentAnimation() : nullptr;
            
            if (animState)
            {
                const std::string currentFilename = fmt::format("{}.ktx2", animState->name);
                const std::string currentHudText1 = fmt::format("{} FPS", currentFps);
                const std::string currentHudText2 = fmt::format("File: {}, Animation: {}, Frames {}", currentFilename, animState->name, animState->frameCount);

                if (currentHudText1 != lastHudText1 || !hudTexture1.has_value())
                {
                    lastHudText1 = currentHudText1;
                    hudTexture1 = bgl::TextRenderer::RenderToTexture(
                        font,
                        currentHudText1,
                        {255, 255, 255, 255},  // White text color
                        {0, 0, 0, 0},          // Transparent background
                        4                      // Padding
                    );
                }

                if (currentHudText2 != lastHudText2 || !hudTexture2.has_value())
                {
                    lastHudText2 = currentHudText2;
                    hudTexture2 = bgl::TextRenderer::RenderToTexture(
                        font,
                        currentHudText2,
                        {255, 255, 255, 255},  // White text color
                        {0, 0, 0, 0},          // Transparent background
                        4                      // Padding
                    );
                }
            }

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            const auto c = (static_cast<int>(time) % 10) / 10.0f;
            glClearColor(c * 0.2f, c * 0.1f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            const auto winSize = g_window->getWindowSize();
            const float aspect = winSize.x / winSize.y;
            const glm::mat4 projection = glm::ortho(
                cameraPosition.x - zoomLevel * aspect,
                cameraPosition.x + zoomLevel * aspect,
                cameraPosition.y - zoomLevel,
                cameraPosition.y + zoomLevel,
                -1.0f, 1.0f
            );

            // Render background
            if (bgProgram != 0)
            {
                glUseProgram(bgProgram);
                const GLint projLoc = glGetUniformLocation(bgProgram, "projection");
                glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
                
                glBindVertexArray(bgQuad.VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            }

            if (animState && animState->texture)
            {
                const auto num_frames = animState->frameCount > 0 ? animState->frameCount : 1;
                constexpr double target_fps = 24.0;

                const double totalFrames = time * target_fps;
                const int currentFrame = static_cast<int>(std::floor(totalFrames)) % num_frames;
                const float tweenFactor = static_cast<float>(totalFrames - std::floor(totalFrames));

                // Render background sprite animation quad
                bgl::renderQuad(spriteQuad, animState->texture->getHandle(), mainProgram, currentFrame, tweenFactor, projection);
            }
            
            // Render snow particles
            if (snowProgram != 0 && snowTexture)
            {
                glUseProgram(snowProgram);
                const GLint projLoc = glGetUniformLocation(snowProgram, "projection");
                const GLint timeLoc = glGetUniformLocation(snowProgram, "time");
                glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
                glUniform1f(timeLoc, static_cast<float>(time));
                
                glBindTextureUnit(1, snowTexture->getHandle());
                glBindVertexArray(snowVAO);
                glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, NUM_SNOW_PARTICLES);
            }

            // Render top-left FPS & VRAM text texture overlay
            if (hudTexture1 && hudTexture1->IsValid())
            {
                bgl::renderTextOverlay(
                    overlayQuad,
                    hudTexture1->GetHandle(),
                    textProgram,
                    hudTexture1->GetWidth(),
                    hudTexture1->GetHeight(),
                    static_cast<uint32_t>(winSize.x),
                    static_cast<uint32_t>(winSize.y),
                    15.0f,
                    15.0f
                );
            }

            if (hudTexture2 && hudTexture2->IsValid() && hudTexture1 && hudTexture1->IsValid())
            {
                bgl::renderTextOverlay(
                    overlayQuad,
                    hudTexture2->GetHandle(),
                    textProgram,
                    hudTexture2->GetWidth(),
                    hudTexture2->GetHeight(),
                    static_cast<uint32_t>(winSize.x),
                    static_cast<uint32_t>(winSize.y),
                    15.0f,
                    15.0f + hudTexture1->GetHeight() + 5.0f
                );
            }
        });

        g_window->run();

        // GPU resources cleanup
        hudTexture1.reset();
        hudTexture2.reset();
        g_character.reset();

        if (mainProgram != 0) glDeleteProgram(mainProgram);
        if (textProgram != 0) glDeleteProgram(textProgram);
        if (bgProgram != 0) glDeleteProgram(bgProgram);
        if (snowProgram != 0) glDeleteProgram(snowProgram);

        glDeleteVertexArrays(1, &snowVAO);
        glDeleteBuffers(1, &snowVBO);
        glDeleteBuffers(1, &quadVBO);
        glDeleteBuffers(1, &quadIBO);

        bgl::destroyQuadMesh(spriteQuad);
        bgl::destroyQuadMesh(overlayQuad);
        bgl::destroyQuadMesh(bgQuad);
        spdlog::info("Released VAO/VBO/IBO buffers and shader programs");
    }
    catch (const std::exception &e)
    {
        spdlog::error("{}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}