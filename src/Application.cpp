// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Application.hpp"
#include "gfx/Sampler.hpp"
#include "gfx/text/TextShaper.hpp"
#include "io/KtxLoader.hpp"
#include "io/ShaderLoader.hpp"

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
        if (event.key == GLFW_KEY_DOWN && event.action == GLFW_PRESS)
        {
            current_char->previousAnimation();
        }
        else if (event.key == GLFW_KEY_TAB && event.action == GLFW_PRESS)
        {
            m_currentCharacterIndex = (m_currentCharacterIndex + 1) % m_characters.size();
        }
        
        if (event.key == GLFW_KEY_RIGHT)
        {
            m_rightPressed = (event.action != GLFW_RELEASE);
        }
        else if (event.key == GLFW_KEY_LEFT)
        {
            m_leftPressed = (event.action != GLFW_RELEASE);
        }
        else if (event.key == GLFW_KEY_SPACE)
        {
            m_spacePressed = (event.action != GLFW_RELEASE);
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
                auto tex = io::loadTexture(file);
                if (tex)
                {
                    GLint layers = 0;
                    glGetTextureLevelParameteriv(tex->getHandle(), 0, GL_TEXTURE_DEPTH, &layers);
                    character->addAnimation(animName, std::move(tex), static_cast<uint32_t>(layers > 0 ? layers : 1));
                }
            }
        }
        m_characters.push_back(character);
    }

    for (size_t i = 0; i < m_characters.size(); ++i) {
        if (m_characters[i]->getName() == "Santa") {
            m_currentCharacterIndex = i;
            break;
        }
    }

    // Load items texture explicitly for world items using automatic texture loader
    auto itemFile = basePath / "assets" / "textures" / "ktx" / "items" / "items.ktx2";
    if (std::filesystem::exists(itemFile))
    {
        m_itemTexture = io::loadTexture(itemFile);
        if (m_itemTexture)
        {
            GLint layers = 0;
            glGetTextureLevelParameteriv(m_itemTexture->getHandle(), 0, GL_TEXTURE_DEPTH, &layers);
            m_itemFrameCount = static_cast<uint32_t>(layers > 0 ? layers : 1);
        }
    }

    // Load TileMap level design from JSON
    auto levelJson = basePath / "assets" / "level.json";
    if (std::filesystem::exists(levelJson))
    {
        m_tileMap.loadFromFile(levelJson);
    }

    auto tilesFile = basePath / "assets" / "textures" / "ktx" / "tiles" / "tiles.ktx2";
    if (std::filesystem::exists(tilesFile))
    {
        m_tileMap.setTexture(io::loadTexture(tilesFile));
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

    constexpr int NUM_SNOW_PARTICLES = 150000;
    std::vector<glm::vec2> snowOffsets(NUM_SNOW_PARTICLES);
    for (int i = 0; i < NUM_SNOW_PARTICLES; ++i)
    {
        float rx = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 40.0f - 20.0f;
        float ry = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 40.0f - 20.0f;
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

    // Generate random items
    if (m_itemFrameCount > 0)
    {
        for (int i = 0; i < 5; ++i)
        {
            float rx = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 16.0f - 8.0f;
            float scale = 0.3f + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 0.2f;
            float tOffset = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 10.0f;
            m_worldItems.push_back({glm::vec2(rx, GROUND_Y - 0.2f), scale, tOffset});
        }
    }
}

void Application::run()
{
    m_audioEngine.init();
    m_audioEngine.playJingleBells();
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

    if (m_lastFrameTime == 0.0) m_lastFrameTime = time;
    double dt = time - m_lastFrameTime;
    m_lastFrameTime = time;

    m_audioEngine.update(static_cast<float>(dt));

    if (!m_characters.empty())
    {
        auto &current_char = m_characters[m_currentCharacterIndex];
        bool isMoving = false;
        glm::vec2 pos = current_char->getPosition();
        if (m_rightPressed)
        {
            pos.x += CHARACTER_SPEED * static_cast<float>(dt);
            current_char->setFlipped(false);
            isMoving = true;
        }
        else if (m_leftPressed)
        {
            pos.x -= CHARACTER_SPEED * static_cast<float>(dt);
            current_char->setFlipped(true);
            isMoving = true;
        }

        if (m_spacePressed && !current_char->isJumping())
        {
            current_char->setVelocityY(JUMP_FORCE);
            current_char->setJumping(true);
            m_audioEngine.playJumpSound();
        }

        if (current_char->isJumping() || pos.y > GROUND_Y)
        {
            float vY = current_char->getVelocityY() - (GRAVITY * static_cast<float>(dt));
            pos.y += vY * static_cast<float>(dt);
            current_char->setVelocityY(vY);

            if (pos.y <= GROUND_Y)
            {
                pos.y = GROUND_Y;
                current_char->setVelocityY(0.0f);
                current_char->setJumping(false);
            }
        }

        current_char->setPosition(pos);

        if (current_char->isJumping())
        {
            current_char->setAnimationByName("Jump");
        }
        else if (isMoving)
        {
            current_char->setAnimationByName("Walk");
        }
        else
        {
            current_char->setAnimationByName("Idle");
        }
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
    m_tileMap.render(m_mainProgram, m_spriteQuad, projection);
    renderSnow(time, projection);
    renderItems(time, projection);
    renderCharacter(time, projection);
    renderUI(time, winSize);
}

void Application::renderBackground(const glm::mat4 &projection)
{
    if (m_bgProgram != 0)
    {
        const GLint projLoc = glGetUniformLocation(m_bgProgram, "projection");
        glProgramUniformMatrix4fv(m_bgProgram, projLoc, 1, GL_FALSE, &projection[0][0]);
        
        glUseProgram(m_bgProgram);

        glBindVertexArray(m_bgQuad.VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
}

void Application::renderCharacter(double time, const glm::mat4 &projection)
{
    auto current_char = !m_characters.empty() ? m_characters[m_currentCharacterIndex] : nullptr;

    if (auto animState = current_char->getCurrentAnimation())
    {
        const auto num_frames = animState->frameCount > 0 ? animState->frameCount : 1;
        constexpr double target_fps = 24.0;

        const double totalFrames = time * target_fps;
        const int currentFrame = static_cast<int>(std::floor(totalFrames)) % num_frames;
        const float tweenFactor = static_cast<float>(totalFrames - std::floor(totalFrames));

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(current_char->getPosition(), 0.0f));
        if (current_char->isFlipped())
        {
            model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
        }

        renderQuad(m_spriteQuad, animState->texture->getHandle(), m_mainProgram, currentFrame, tweenFactor, projection, model);
    }
}

void Application::renderSnow(double time, const glm::mat4 &projection)
{
    if (m_snowProgram != 0)
    {
        const GLint projLoc = glGetUniformLocation(m_snowProgram, "projection");
        const GLint timeLoc = glGetUniformLocation(m_snowProgram, "time");
        glProgramUniformMatrix4fv(m_snowProgram, projLoc, 1, GL_FALSE, &projection[0][0]);
        glProgramUniform1f(m_snowProgram, timeLoc, static_cast<float>(time));

        glUseProgram(m_snowProgram);

        glBindVertexArray(m_snowVAO);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, 150000);
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

void Application::renderItems(double time, const glm::mat4 &projection)
{
    if (!m_itemTexture || !m_itemTexture->isValid() || m_itemFrameCount == 0 || m_worldItems.empty())
        return;

    for (const auto &item : m_worldItems)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(item.pos, 0.0f));
        model = glm::scale(model, glm::vec3(item.scale));

        const double itemTime = time + item.timeOffset;
        const double target_fps = 10.0;
        const double totalFrames = itemTime * target_fps;
        const int currentFrame = static_cast<int>(std::floor(totalFrames)) % m_itemFrameCount;

        renderQuad(m_spriteQuad, m_itemTexture->getHandle(), m_mainProgram, currentFrame, 0.0f, projection, model);
    }
}

} // namespace bgl
