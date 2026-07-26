// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include "Character.hpp"
#include "Window.hpp"
#include "gfx/Font.hpp"
#include "gfx/Graphics.hpp"
#include "gfx/Text_renderer.hpp"

#include <glad/gl.h>
#include <glm/vec2.hpp>
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

    void renderFrame(double time);
    void renderBackground(const glm::mat4 &projection);
    void renderCharacter(double time, const glm::mat4 &projection);
    void renderSnow(double time, const glm::mat4 &projection);
    void renderUI(double time, const glm::vec2 &winSize);

    std::unique_ptr<Window> m_window;
    std::vector<std::shared_ptr<Character>> m_characters;
    size_t m_currentCharacterIndex = 0;

    std::optional<Font> m_font;

    float m_zoomLevel = 1.0f;
    glm::vec2 m_cameraPosition{0.0f, 0.0f};
    bool m_isPanning = false;
    glm::vec2 m_lastMousePos{0.0f, 0.0f};

    GLuint m_mainProgram = 0;
    GLuint m_textProgram = 0;
    GLuint m_bgProgram = 0;
    GLuint m_snowProgram = 0;

    std::unique_ptr<gfx::Texture2DArray> m_snowTexture;

    QuadMesh m_spriteQuad;
    QuadMesh m_overlayQuad;
    QuadMesh m_bgQuad;

    GLuint m_snowVAO = 0;
    GLuint m_snowVBO = 0;
    GLuint m_quadVBO = 0;
    GLuint m_quadIBO = 0;

    double m_lastFpsTime = 0.0;
    int m_frameCounter = 0;
    int m_currentFps = 60;
    std::string m_lastHudText1;
    std::string m_lastHudText2;
    std::optional<TextTexture> m_hudTexture1;
    std::optional<TextTexture> m_hudTexture2;
};

} // namespace bgl
