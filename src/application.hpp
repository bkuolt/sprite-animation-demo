// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "character.hpp"
#include "windowing/window.hpp"
#include "gfx/camera.hpp"
#include "gfx/font.hpp"
#include "gfx/graphics.hpp"
#include "gfx/hud.hpp"
#include "gfx/textRenderer.hpp"

#include <glad/gl.h>
#include <glm/vec2.hpp>
#include <entt/entt.hpp>
#include "events/event.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bgl
{

class Application
{
  public:
    Application();
    ~Application();

    void run();

  private:
    void initAssets();
    void initShaders();
    void initMeshes();
    void setupCallbacks();

    void onKeyEvent(const events::KeyEvent& event);
    void onScrollEvent(const events::ScrollEvent& event);
    void onCursorPosEvent(const events::MouseMovedEvent& event);
    void onMouseButtonEvent(const events::MouseButtonEvent& event);

    void renderFrame(double time);
    void renderBackground(const glm::mat4 &projection);
    void renderCharacter(double time, const glm::mat4 &projection);
    void renderSnow(double time, const glm::mat4 &projection);
    void renderUI(double time, const glm::vec2 &winSize);

    entt::dispatcher m_eventDispatcher;
    std::unique_ptr<bgl::window::Window> m_window;
    std::vector<std::shared_ptr<Character>> m_characters;
    size_t m_currentCharacterIndex = 0;

    std::optional<Font> m_font;

    gfx::Camera m_camera;
    gfx::Hud m_hud;

    GLuint m_mainProgram = 0;
    GLuint m_textProgram = 0;
    GLuint m_bgProgram = 0;
    GLuint m_snowProgram = 0;

    std::unique_ptr<gfx::Texture2DArray> m_snowTexture;

    QuadMesh m_spriteQuad;
    QuadMesh m_overlayQuad;
    QuadMesh m_bgQuad;

    std::unique_ptr<gfx::Sampler> m_defaultSampler;

    GLuint m_snowVAO = 0;
    GLuint m_snowVBO = 0;
    GLuint m_quadVBO = 0;
    GLuint m_quadIBO = 0;

    double m_lastFpsTime = 0.0;
    int m_frameCounter = 0;
    int m_currentFps = 60;
};

} // namespace bgl
