// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "application.hpp"
#include "gfx/sampler.hpp"
#include "gfx/textShaper.hpp"
#include "io/ktxLoader.hpp"
#include "io/shaderLoader.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace bgl
{

Application::Application()
{
    m_window = std::make_unique<bgl::window::Window>();
    m_window->setEventDispatcher(&m_eventDispatcher);

    setupCallbacks();
    initAssets();
    initShaders();
    initMeshes();

    m_defaultSampler = bgl::CreateDefaultSampler();
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
    m_eventDispatcher.sink<events::ScrollEvent>().connect<&Application::onScrollEvent>(this);
    m_eventDispatcher.sink<events::MouseButtonEvent>().connect<&Application::onMouseButtonEvent>(this);
    m_eventDispatcher.sink<events::MouseMovedEvent>().connect<&Application::onCursorPosEvent>(this);
    m_eventDispatcher.sink<events::KeyEvent>().connect<&Application::onKeyEvent>(this);
}

void Application::onKeyEvent(const events::KeyEvent &event)
{
    if (event.key == GLFW_KEY_R && event.action == GLFW_PRESS)
    {
        m_camera.reset();
    }

    if (!m_characters.empty())
    {
        auto &current_char = m_characters[m_currentCharacterIndex];
        if ((event.key == GLFW_KEY_SPACE || event.key == GLFW_KEY_UP) && event.action == GLFW_PRESS)
        {
            current_char->nextAnimation();
        }
        else if (event.key == GLFW_KEY_DOWN && event.action == GLFW_PRESS)
        {
            current_char->previousAnimation();
        }
        else if (event.key == GLFW_KEY_RIGHT && event.action == GLFW_PRESS)
        {
            m_currentCharacterIndex = (m_currentCharacterIndex + 1) % m_characters.size();
        }
        else if (event.key == GLFW_KEY_LEFT && event.action == GLFW_PRESS)
        {
            m_currentCharacterIndex =
                (m_currentCharacterIndex > 0) ? (m_currentCharacterIndex - 1) : (m_characters.size() - 1);
        }
    }
}

void Application::onScrollEvent(const events::ScrollEvent &event)
{
    m_camera.handleScroll(event.yoffset);
}

void Application::onCursorPosEvent(const events::MouseMovedEvent &event)
{
    const auto winSize = m_window->getWindowSize();
    m_camera.handleCursorPos(event.xpos, event.ypos, static_cast<float>(winSize.x), static_cast<float>(winSize.y));
}

void Application::onMouseButtonEvent(const events::MouseButtonEvent &event)
{
    m_camera.handleMouseButton(event.button, event.action);
}


void Application::initAssets()
{
    const auto binaryPath = std::filesystem::read_symlink("/proc/self/exe");
    const auto basePath = binaryPath.parent_path();

    // Load a bold sans-serif font
    m_font = Font::LoadSystemFont("sans-serif:bold", 36);

    const auto jsonPath = basePath / "assets" / "animations.json";
    std::ifstream f(jsonPath);
    if (!f.is_open())
    {
        spdlog::error("Failed to open animations.json at {}", jsonPath.string());
        return;
    }

    nlohmann::json data;
    try
    {
        data = nlohmann::json::parse(f);
    }
    catch (const nlohmann::json::parse_error &e)
    {
        spdlog::error("JSON parse error: {}", e.what());
        return;
    }

    if (!data.contains("characters"))
        return;

    for (const auto &charNode : data["characters"])
    {
        std::string charName = charNode.value("name", "Unknown");
        auto character = std::make_shared<Character>(charName);

        if (charNode.contains("animations"))
        {
            for (const auto &animNode : charNode["animations"])
            {
                std::string animName = animNode.value("name", "Unknown");
                std::string animFile = animNode.value("file", "");
                if (animFile.empty())
                    continue;

                auto file = basePath / "assets" / animFile;
                io::KtxLoader loader(file, KTX_TTF_BC3_RGBA);
                auto tex = loader.upload();
                GLint layers = 0;
                glGetTextureLevelParameteriv(tex->getHandle(), 0, GL_TEXTURE_DEPTH, &layers);
                character->addAnimation(animName, std::move(tex), static_cast<uint32_t>(layers > 0 ? layers : 1));
            }
        }
        m_characters.push_back(character);
    }
}

void Application::initShaders()
{
    const auto binaryPath = std::filesystem::read_symlink("/proc/self/exe");
    const auto basePath = binaryPath.parent_path();

    const auto mainVsSpv = io::LoadSPIRVShaderFromFile(basePath / "assets" / "shaders" / "main.vert.spv");
    const auto mainFsSpv = io::LoadSPIRVShaderFromFile(basePath / "assets" / "shaders" / "main.frag.spv");
    m_mainProgram = io::CreateShaderProgramFromSPIRV(mainVsSpv, mainFsSpv);

    const auto textVsSpv = io::LoadSPIRVShaderFromFile(basePath / "assets" / "shaders" / "text.vert.spv");
    const auto textFsSpv = io::LoadSPIRVShaderFromFile(basePath / "assets" / "shaders" / "text.frag.spv");
    m_textProgram = io::CreateShaderProgramFromSPIRV(textVsSpv, textFsSpv);

#ifdef BGL_ENABLE_GLSL_LOADER
    const auto srcPath = basePath / "assets" / "shaders";
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

    if (m_defaultSampler)
    {
        m_defaultSampler->bind(0);
    }

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
