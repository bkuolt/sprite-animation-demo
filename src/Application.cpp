// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "Application.hpp"
#include "gfx/Text_shaper.hpp"
#include "io/KtxLoader.hpp"
#include "io/PngLoader.hpp"
#include "io/ShaderLoader.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fmt/format.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <spdlog/spdlog.h>

namespace bgl
{

Application::Application()
{
    m_window = std::make_unique<Window>();

    setupCallbacks();
    initAssets();
    initShaders();
    initMeshes();
}

Application::~Application()
{
    m_characters.clear();

    if (m_mainProgram != 0)
        glDeleteProgram(m_mainProgram);
    if (m_textProgram != 0)
        glDeleteProgram(m_textProgram);
    if (m_bgProgram != 0)
        glDeleteProgram(m_bgProgram);
    if (m_snowProgram != 0)
        glDeleteProgram(m_snowProgram);

    if (m_snowVAO != 0)
        glDeleteVertexArrays(1, &m_snowVAO);
    if (m_snowVBO != 0)
        glDeleteBuffers(1, &m_snowVBO);
    if (m_quadVBO != 0)
        glDeleteBuffers(1, &m_quadVBO);
    if (m_quadIBO != 0)
        glDeleteBuffers(1, &m_quadIBO);

    bgl::destroyQuadMesh(m_spriteQuad);
    bgl::destroyQuadMesh(m_overlayQuad);
    bgl::destroyQuadMesh(m_bgQuad);

    spdlog::info("Released VAO/VBO/IBO buffers and shader programs");
}

void Application::setupCallbacks()
{
    m_window->setScrollCallback([this](double /*xoffset*/, double yoffset) { m_camera.handleScroll(yoffset); });

    m_window->setMouseButtonCallback(
        [this](int button, int action, int /*mods*/) { m_camera.handleMouseButton(button, action); });

    m_window->setCursorPosCallback([this](double xpos, double ypos) {
        const auto winSize = m_window->getWindowSize();
        m_camera.handleCursorPos(xpos, ypos, static_cast<float>(winSize.x), static_cast<float>(winSize.y));
    });

    m_window->setKeyCallback(
        [this](int key, int /*scancode*/, int action, int /*mods*/)
        {
            if (key == GLFW_KEY_R && action == GLFW_PRESS)
            {
                m_camera.reset();
            }

            if (!m_characters.empty())
            {
                auto &current_char = m_characters[m_currentCharacterIndex];
                if ((key == GLFW_KEY_SPACE || key == GLFW_KEY_UP) && action == GLFW_PRESS)
                {
                    current_char->nextAnimation();
                }
                else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
                {
                    current_char->previousAnimation();
                }
                else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
                {
                    m_currentCharacterIndex = (m_currentCharacterIndex + 1) % m_characters.size();
                }
                else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
                {
                    m_currentCharacterIndex =
                        (m_currentCharacterIndex > 0) ? (m_currentCharacterIndex - 1) : (m_characters.size() - 1);
                }
            }
        });
}

void Application::initAssets()
{
    const auto binaryPath = std::filesystem::read_symlink("/proc/self/exe");
    const auto basePath = binaryPath.parent_path();

    // Load a bold sans-serif font
    m_font = Font::LoadSystemFont("sans-serif:bold", 36);

    auto hero = std::make_shared<Character>("Hero");
    auto villain = std::make_shared<Character>("Villain");

    const std::array fileNames{std::make_pair("Idle", basePath / "assets" / "idle.ktx2"),
                               std::make_pair("Walk", basePath / "assets" / "walk.ktx2"),
                               std::make_pair("Jump", basePath / "assets" / "jump.ktx2"),
                               std::make_pair("Run", basePath / "assets" / "run.ktx2"),
                               std::make_pair("Slide", basePath / "assets" / "slide.ktx2"),
                               std::make_pair("Dead", basePath / "assets" / "dead.ktx2")};

    for (const auto &[animName, file] : fileNames)
    {
        io::KtxLoader loaderHero(file, KTX_TTF_BC3_RGBA);
        auto texHero = loaderHero.upload();
        GLint layersHero = 0;
        glGetTextureLevelParameteriv(texHero->getHandle(), 0, GL_TEXTURE_DEPTH, &layersHero);
        hero->addAnimation(animName, std::move(texHero), static_cast<uint32_t>(layersHero > 0 ? layersHero : 1));

        io::KtxLoader loaderVillain(file, KTX_TTF_BC3_RGBA);
        auto texVillain = loaderVillain.upload();
        GLint layersVillain = 0;
        glGetTextureLevelParameteriv(texVillain->getHandle(), 0, GL_TEXTURE_DEPTH, &layersVillain);
        villain->addAnimation(animName, std::move(texVillain),
                              static_cast<uint32_t>(layersVillain > 0 ? layersVillain : 1));
    }

        m_characters.push_back(hero);
        m_characters.push_back(villain);
}

void Application::initShaders()
{
    const auto binaryPath = std::filesystem::read_symlink("/proc/self/exe");
    const auto basePath = binaryPath.parent_path();

    const auto mainVsSpv = io::LoadSPIRVShaderFromFile(basePath / "assets" / "main.vert.spv");
    const auto mainFsSpv = io::LoadSPIRVShaderFromFile(basePath / "assets" / "main.frag.spv");
    m_mainProgram = io::CreateShaderProgramFromSPIRV(mainVsSpv, mainFsSpv);

    const auto textVsSpv = io::LoadSPIRVShaderFromFile(basePath / "assets" / "text.vert.spv");
    const auto textFsSpv = io::LoadSPIRVShaderFromFile(basePath / "assets" / "text.frag.spv");
    m_textProgram = io::CreateShaderProgramFromSPIRV(textVsSpv, textFsSpv);

#ifdef BGL_ENABLE_GLSL_LOADER
    const auto srcPath = basePath / ".." / ".." / "src" / "shaders";
    const auto bgVsSrc = io::LoadShaderFromFile(srcPath / "background.vs");
    const auto bgFsSrc = io::LoadShaderFromFile(srcPath / "background.fs");
    m_bgProgram = io::CreateShaderProgramFromGLSL(bgVsSrc, bgFsSrc);

    const auto snowVsSrc = io::LoadShaderFromFile(srcPath / "snow.vs");
    const auto snowFsSrc = io::LoadShaderFromFile(srcPath / "snow.fs");
    m_snowProgram = io::CreateShaderProgramFromGLSL(snowVsSrc, snowFsSrc);
#endif
}

void Application::initMeshes()
{
    m_spriteQuad = create2DQuad();
    m_overlayQuad = createOverlayQuad();
    m_bgQuad = create2DQuad();

    constexpr int NUM_SNOW_PARTICLES = 1500;
    std::vector<glm::vec2> snowOffsets(NUM_SNOW_PARTICLES);
    for (int i = 0; i < NUM_SNOW_PARTICLES; ++i)
    {
        float rx = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 20.0f - 10.0f;
        float ry = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 20.0f - 10.0f;
        snowOffsets[i] = glm::vec2(rx, ry);
    }

    glCreateBuffers(1, &m_snowVBO);
    glNamedBufferStorage(m_snowVBO, snowOffsets.size() * sizeof(glm::vec2), snowOffsets.data(), 0);

    glCreateVertexArrays(1, &m_snowVAO);

    constexpr std::array<glm::vec2, 4> baseQuad = {glm::vec2(-1.0f, -1.0f), glm::vec2(1.0f, -1.0f),
                                                   glm::vec2(1.0f, 1.0f), glm::vec2(-1.0f, 1.0f)};
    constexpr std::array<GLuint, 6> baseIndices = {0, 1, 2, 2, 3, 0};

    glCreateBuffers(1, &m_quadVBO);
    glCreateBuffers(1, &m_quadIBO);
    glNamedBufferStorage(m_quadVBO, baseQuad.size() * sizeof(glm::vec2), baseQuad.data(), 0);
    glNamedBufferStorage(m_quadIBO, baseIndices.size() * sizeof(GLuint), baseIndices.data(), 0);

    glVertexArrayVertexBuffer(m_snowVAO, 0, m_quadVBO, 0, sizeof(glm::vec2));
    glVertexArrayVertexBuffer(m_snowVAO, 1, m_snowVBO, 0, sizeof(glm::vec2));
    glVertexArrayElementBuffer(m_snowVAO, m_quadIBO);

    glEnableVertexArrayAttrib(m_snowVAO, 0);
    glVertexArrayAttribFormat(m_snowVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_snowVAO, 0, 0);

    glEnableVertexArrayAttrib(m_snowVAO, 1);
    glVertexArrayAttribFormat(m_snowVAO, 1, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_snowVAO, 1, 1);
    glVertexArrayBindingDivisor(m_snowVAO, 1, 1);
}

void Application::run()
{
    m_window->setRenderCallback([this](double time) { renderFrame(time); });
    m_window->run();
}

void Application::renderFrame(double time)
{
    m_frameCounter++;
    if (time - m_lastFpsTime >= 0.2)
    {
        m_currentFps = static_cast<int>(std::round(m_frameCounter / (time - m_lastFpsTime)));
        m_frameCounter = 0;
        m_lastFpsTime = time;
    }

    const auto winSize = m_window->getWindowSize();
    const float aspect = winSize.x / winSize.y;
    const glm::mat4 projection = m_camera.getProjectionMatrix(aspect);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const auto c = (static_cast<int>(time) % 10) / 10.0f;
    glClearColor(c * 0.2f, c * 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderBackground(projection);
    renderSnow(time, projection);
    renderCharacter(time, projection);
    renderUI(time, winSize);
}

void Application::renderBackground(const glm::mat4 &projection)
{
    if (m_bgProgram != 0)
    {
        glUseProgram(m_bgProgram);
        const GLint projLoc = glGetUniformLocation(m_bgProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(m_bgQuad.VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
}

void Application::renderCharacter(double time, const glm::mat4 &projection)
{
    auto current_char = !m_characters.empty() ? m_characters[m_currentCharacterIndex] : nullptr;
    const auto *animState = current_char ? current_char->getCurrentAnimation() : nullptr;

    if (animState && animState->texture)
    {
        const auto num_frames = animState->frameCount > 0 ? animState->frameCount : 1;
        constexpr double target_fps = 24.0;

        const double totalFrames = time * target_fps;
        const int currentFrame = static_cast<int>(std::floor(totalFrames)) % num_frames;
        const float tweenFactor = static_cast<float>(totalFrames - std::floor(totalFrames));

        renderQuad(m_spriteQuad, animState->texture->getHandle(), m_mainProgram, currentFrame, tweenFactor, projection);
    }
}

void Application::renderSnow(double time, const glm::mat4 &projection)
{
    if (m_snowProgram != 0)
    {
        glUseProgram(m_snowProgram);
        const GLint projLoc = glGetUniformLocation(m_snowProgram, "projection");
        const GLint timeLoc = glGetUniformLocation(m_snowProgram, "time");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
        glUniform1f(timeLoc, static_cast<float>(time));

        glBindVertexArray(m_snowVAO);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, 1500);
    }
}

void Application::renderUI(double /*time*/, const glm::vec2 &winSize)
{
    auto current_char = !m_characters.empty() ? m_characters[m_currentCharacterIndex] : nullptr;
    if (m_font)
    {
        m_hud.updateAndRender(*m_font, m_textProgram, m_overlayQuad, winSize, m_currentFps, current_char);
    }
}

} // namespace bgl
