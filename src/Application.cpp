// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Application.hpp"
#include "Character.hpp"
#include "audio/AudioEngine.hpp"
#include "gfx/Camera.hpp"
#include "gfx/Hud.hpp"
#include "gfx/Sampler.hpp"
#include "gfx/TileMap.hpp"
#include "gfx/text/Font.hpp"
#include "gfx/text/TextShaper.hpp"
#include "io/ShaderLoader.hpp"
#include "io/TextureLoader.hpp"
#include "windowing/Window.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <nlohmann/json.hpp>
#include <random>
#include <spdlog/spdlog.h>

namespace bgl
{
namespace
{
/// Returns the directory containing the running executable.
/// Cached so repeated calls don't re-stat /proc.
[[nodiscard]] const std::filesystem::path &getExecutableDir()
{
    static const std::filesystem::path dir =
        std::filesystem::read_symlink("/proc/self/exe").parent_path();
    return dir;
}

/// Computes the current animation frame index from wall-clock time.
[[nodiscard]] int computeAnimFrame(double time, double fps, int frameCount) noexcept
{
    if (frameCount <= 0) return 0;
    return static_cast<int>(std::floor(time * fps)) % frameCount;
}

/// Computes the tween factor (0..1) between the current and next frame.
[[nodiscard]] float computeAnimTween(double time, double fps) noexcept
{
    const double totalFrames = time * fps;
    return static_cast<float>(totalFrames - std::floor(totalFrames));
}
} // namespace


Application::Application()
{
    m_window       = std::make_unique<bgl::window::Window>();
    m_camera       = std::make_unique<bgl::gfx::Camera>();
    m_hud          = std::make_unique<bgl::gfx::Hud>();
    m_tileMap      = std::make_unique<bgl::gfx::TileMap>();
    m_audioEngine  = std::make_unique<bgl::audio::AudioEngine>();

    m_window->setEventDispatcher(&m_eventDispatcher);

    setupCallbacks();
    initAssets();
    initShaders();
    initMeshes();

    m_defaultSampler = bgl::CreateDefaultSampler();
}

Application::~Application()
{
    // QuadMesh members are RAII — they release GPU resources automatically.
    // Explicit program + buffer cleanup for handles not wrapped in RAII types.
    if (m_mainProgram)  glDeleteProgram(m_mainProgram);
    if (m_textProgram)  glDeleteProgram(m_textProgram);
    if (m_bgProgram)    glDeleteProgram(m_bgProgram);
    if (m_snowProgram)  glDeleteProgram(m_snowProgram);

    if (m_snowVAO) glDeleteVertexArrays(1, &m_snowVAO);
    if (m_snowVBO) glDeleteBuffers(1, &m_snowVBO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
    if (m_quadIBO) glDeleteBuffers(1, &m_quadIBO);

    spdlog::info("Application resources released.");
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
        m_camera->reset();
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
    m_camera->handleScroll(event.yoffset);
}

void Application::onCursorPosEvent(const events::MouseMovedEvent &event)
{
    const auto winSize = m_window->getWindowSize();
    m_camera->handleCursorPos(event.xpos, event.ypos, static_cast<float>(winSize.x), static_cast<float>(winSize.y));
}

void Application::onMouseButtonEvent(const events::MouseButtonEvent &event)
{
    m_camera->handleMouseButton(event.button, event.action);
}


void Application::initAssets()
{
    const auto &basePath = getExecutableDir();

    // Load a bold sans-serif font
    m_font = std::make_unique<Font>(Font::LoadSystemFont("sans-serif:bold", 36));

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
                    // Use layerCount() — no driver round-trip needed.
                    const uint32_t frames = std::max(1u, tex->layerCount());
                    character->addAnimation(animName, std::move(tex), frames);
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
            m_itemFrameCount = std::max(1u, m_itemTexture->layerCount());
        }
    }

    // Load TileMap level design from JSON
    auto levelJson = basePath / "assets" / "level.json";
    if (std::filesystem::exists(levelJson))
    {
        m_tileMap->loadFromFile(levelJson);
    }

    auto tilesFile = basePath / "assets" / "textures" / "ktx" / "tiles" / "tiles.ktx2";
    if (std::filesystem::exists(tilesFile))
    {
        m_tileMap->setTexture(io::loadTexture(tilesFile));
    }
}

void Application::initShaders()
{
    const auto &basePath = getExecutableDir();

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
    {
        // Use a properly seeded Mersenne Twister — std::rand() is non-uniform and
        // not seeded here, producing identical sequences across runs.
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(-20.0f, 20.0f);
        for (auto &offset : snowOffsets)
        {
            offset = glm::vec2(dist(rng), dist(rng));
        }
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

    // Generate random world items using a seeded RNG.
    if (m_itemFrameCount > 0)
    {
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> xDist(-8.0f, 8.0f);
        std::uniform_real_distribution<float> scaleDist(0.3f, 0.5f);
        std::uniform_real_distribution<float> timeDist(0.0f, 10.0f);

        for (int i = 0; i < 5; ++i)
        {
            m_worldItems.push_back({
                glm::vec2(xDist(rng), kGroundY - 0.2f),
                scaleDist(rng),
                timeDist(rng)
            });
        }
    }
}

void Application::run()
{
    // AudioEngine is initialised in its constructor — start playing immediately.
    m_audioEngine->playJingleBells();
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

    m_audioEngine->update(static_cast<float>(dt));

    if (!m_characters.empty())
    {
        auto &current_char = m_characters[m_currentCharacterIndex];
        bool isMoving = false;
        glm::vec2 pos = current_char->getPosition();
        if (m_rightPressed)
        {
            pos.x += kCharacterSpeed * static_cast<float>(dt);
            current_char->setFlipped(false);
            isMoving = true;
        }
        else if (m_leftPressed)
        {
            pos.x -= kCharacterSpeed * static_cast<float>(dt);
            current_char->setFlipped(true);
            isMoving = true;
        }

        if (m_spacePressed && !current_char->isJumping())
        {
            current_char->setVelocityY(kJumpForce);
            current_char->setJumping(true);
            m_audioEngine->playJumpSound();
        }

        if (current_char->isJumping() || pos.y > kGroundY)
        {
            float vY = current_char->getVelocityY() - (kGravity * static_cast<float>(dt));
            pos.y += vY * static_cast<float>(dt);
            current_char->setVelocityY(vY);

            if (pos.y <= kGroundY)
            {
                pos.y = kGroundY;
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
    const glm::mat4 projection = m_camera->getProjectionMatrix(aspect);

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
    m_tileMap->render(m_mainProgram, m_spriteQuad, projection);
    renderSnow(time, projection);
    renderItems(time, projection);
    renderCharacter(time, projection);
    renderUI(time, winSize);
}

void Application::renderBackground(const glm::mat4 &projection)
{
    if (m_bgProgram != 0)
    {
        // glProgramUniform* is DSA — no glUseProgram needed before setting uniforms.
        glProgramUniformMatrix4fv(m_bgProgram, glGetUniformLocation(m_bgProgram, "projection"),
                                  1, GL_FALSE, &projection[0][0]);
        glUseProgram(m_bgProgram);
        glBindVertexArray(m_bgQuad.getVAO());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
}

void Application::renderCharacter(double time, const glm::mat4 &projection)
{
    auto current_char = !m_characters.empty() ? m_characters[m_currentCharacterIndex] : nullptr;

    if (auto *animState = current_char->getCurrentAnimation())
    {
        constexpr double kFps = 24.0;
        const int   currentFrame = computeAnimFrame(time, kFps, static_cast<int>(animState->frameCount));
        const float tweenFactor  = computeAnimTween(time, kFps);

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
        m_hud->updateAndRender(*m_font, m_textProgram, m_overlayQuad, winSize, m_currentFps, current_char.get());
    }
}

void Application::renderItems(double time, const glm::mat4 &projection)
{
    if (!m_itemTexture || !m_itemTexture->isValid() || m_itemFrameCount == 0 || m_worldItems.empty())
        return;

    constexpr double kFps = 10.0;
    const GLuint texHandle = m_itemTexture->getHandle();

    for (const auto &item : m_worldItems)
    {
        glm::mat4 model =
            glm::scale(
                glm::translate(glm::mat4(1.0f), glm::vec3(item.pos, 0.0f)),
                glm::vec3(item.scale));

        const int frame = computeAnimFrame(time + item.timeOffset, kFps, static_cast<int>(m_itemFrameCount));
        renderQuad(m_spriteQuad, texHandle, m_mainProgram, frame, 0.0f, projection, model);
    }
}

} // namespace bgl
