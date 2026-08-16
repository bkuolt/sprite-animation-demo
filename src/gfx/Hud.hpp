// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "Graphics.hpp"
#include "text/Font.hpp"
#include "text/TextRenderer.hpp"

#include <glm/vec2.hpp>
#include <optional>
#include <string>

namespace bgl { class Character; struct AnimationState; }

namespace bgl::gfx
{

struct CustomTextItem
{
    std::string text;
    float x{15.0f};
    float y{15.0f};
    glm::u8vec4 color{255, 255, 255, 255};
};

class Hud
{
  public:
    Hud() = default;

    /// Adds a custom text overlay line (e.g. from Lua scripts).
    void addText(std::string text, float x, float y, glm::u8vec4 color = {255, 255, 255, 255});

    /// Clears all custom text overlays.
    void clearCustomTexts();

    /// Renders the HUD overlay with FPS, animation info, and custom Lua texts.
    /// @param modelName The name of the currently rendered 3D model.
    void updateAndRender(const Font &font, GLuint textProgram, const QuadMesh &overlayQuad,
                         const glm::vec2 &winSize, int currentFps, const std::string &modelName);

  private:
    std::string m_lastHudText1;
    std::string m_lastHudText2;
    std::optional<TextTexture> m_hudTexture1;
    std::optional<TextTexture> m_hudTexture2;
    std::optional<TextTexture> m_hudTexture3;

    std::vector<CustomTextItem> m_customTexts;
    std::vector<TextTexture> m_customTextTextures;
};

} // namespace bgl::gfx
