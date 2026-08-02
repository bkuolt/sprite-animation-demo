// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "Character.hpp"
#include "events/Event.hpp"
#include "gfx/Camera.hpp"
#include "gfx/Graphics.hpp"
#include "gfx/Hud.hpp"
#include "gfx/text/Font.hpp"
#include "gfx/text/TextRenderer.hpp"
#include "windowing/Window.hpp"
#include <entt/entt.hpp>
#include <glad/gl.h>
#include <glm/vec2.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bgl
{

/**
 * @brief Main engine application class managing rendering loop, windowing, assets, input events, and entities.
 */
class Application
{
  public:
    /**
     * @brief Initializes application resources, window, events, and default graphics state.
     */
    Application();

    /**
     * @brief Cleans up allocated OpenGL resources, programs, and buffers.
     */
    ~Application();

    /**
     * @brief Enters the main application execution and render loop.
     */
    void run();

  private:
    /** @brief Loads font and character animation data from asset directory. */
    void initAssets();

    /** @brief Compiles and links SPIR-V/GLSL shader programs. */
    void initShaders();

    /** @brief Constructs 2D quads and particle buffer objects. */
    void initMeshes();

    /** @brief Connects event dispatcher sinks to application event handlers. */
    void setupCallbacks();

    /** @brief Input event callbacks. */
    void onKeyEvent(const events::KeyEvent &event);
    void onScrollEvent(const events::ScrollEvent &event);
    void onCursorPosEvent(const events::MouseMovedEvent &event);
    void onMouseButtonEvent(const events::MouseButtonEvent &event);

    /** @brief Per-frame update and rendering pipeline. */
    void renderFrame(double time);
    void renderBackground(const glm::mat4 &projection);
    void renderCharacter(double time, const glm::mat4 &projection);
    void renderSnow(double time, const glm::mat4 &projection);
    void renderUI(double time, const glm::vec2 &winSize);

    entt::dispatcher m_eventDispatcher;
    std::unique_ptr<bgl::window::Window> m_window;
    std::vector<std::shared_ptr<Character>> m_characters;
    size_t m_currentCharacterIndex{0};

    std::optional<Font> m_font;

    gfx::Camera m_camera;
    gfx::Hud m_hud;

    GLuint m_mainProgram{0};
    GLuint m_textProgram{0};
    GLuint m_bgProgram{0};
    GLuint m_snowProgram{0};

    std::unique_ptr<gfx::Texture2DArray> m_snowTexture;

    QuadMesh m_spriteQuad;
    QuadMesh m_overlayQuad;
    QuadMesh m_bgQuad;

    std::unique_ptr<gfx::Sampler> m_defaultSampler;

    GLuint m_snowVAO{0};
    GLuint m_snowVBO{0};
    GLuint m_quadVBO{0};
    GLuint m_quadIBO{0};

    double m_lastFpsTime{0.0};
    double m_lastFrameTime{0.0};
    int m_frameCounter{0};
    int m_currentFps{60};

    bool m_leftPressed{false};
    bool m_rightPressed{false};
    bool m_spacePressed{false};

    static constexpr float CHARACTER_SPEED = 2.0f;
    static constexpr float GRAVITY = 15.0f;
    static constexpr float JUMP_FORCE = 8.0f;
    static constexpr float GROUND_Y = 0.0f;
};

} // namespace bgl
